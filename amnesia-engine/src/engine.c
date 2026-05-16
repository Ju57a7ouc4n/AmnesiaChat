#include "engine.h"

static Session sessions[MAX_SESSIONS];
static uint32_t current_query_chat_id = 0;
static int faro_fd = -1;
static uint32_t incoming_chat_counter = 10000;

void engine_init(void) {
    for (int i = 0; i < MAX_SESSIONS; i++) {
        sessions[i].active = 0;
        sessions[i].sock_fd = -1;
        sessions[i].arena = NULL;
        sessions[i].handshake_state = 0;
    }
}

static int find_session_by_id(uint32_t chat_id) {
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (sessions[i].active && sessions[i].chat_id == chat_id) {
            return i;
        }
    }
    return -1;
}

static int find_session_by_sock(int sock_fd) {
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (sessions[i].active && sessions[i].sock_fd == sock_fd) {
            return i;
        }
    }
    return -1;
}

static int get_free_session_slot(void) {
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (!sessions[i].active) {
            return i;
        }
    }
    return -1;
}

static void on_history_decrypted(const char* text, int is_fragmented) {
    ipc_send_response("HIST_CHUNK", current_query_chat_id, text);
    
    if (is_fragmented == 0) {
        ipc_send_response("HIST_END", current_query_chat_id, "EOF");
    }
}

void engine_panic(void) {
    if (faro_fd != -1) {
        netty_close(faro_fd);
    }
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (sessions[i].active) {
            netty_close(sessions[i].sock_fd);
            endChat(sessions[i].arena);
            sessions[i].active = 0;
        }
    }
    ipc_send_response("PANIC_OK", 0, "DESTROYED");
    exit(0);
}

int engine_get_faro_fd(void) {
    return faro_fd;
}

void engine_accept_faro(void) {
    char client_ip[64] = {0};
    int new_sock = netty_accept_client(faro_fd, client_ip);
    
    if (new_sock >= 0) {
        int slot = get_free_session_slot();
        if (slot == -1) {
            netty_close(new_sock);
            return;
        }

        uint32_t new_id = incoming_chat_counter++;
        pChat new_arena = newChat(new_id);
        
        unsigned char pk[crypto_kem_mlkem768_PUBLICKEYBYTES];
        crypto_kem_mlkem768_keypair(pk, sessions[slot].sk);

        netty_send_data(new_sock, pk, sizeof(pk));

        sessions[slot].chat_id = new_id;
        sessions[slot].sock_fd = new_sock;
        sessions[slot].arena = new_arena;
        sessions[slot].active = 1;
        sessions[slot].handshake_state = STATE_WAIT_CT; 

        ipc_send_response("INC_CHAT", new_id, client_ip);
    }
}

void engine_handle_ipc(ParsedCommand* cmd) {
    if (cmd == NULL) return;

    if (cmd->type == CMD_PANIC) {
        engine_panic();
        return;
    }

    int slot;
    int s_idx;

    switch (cmd->type) {
        case CMD_START_FARO:
            if (faro_fd != -1) {
                ipc_log_error("FARO_ALREADY_RUNNING");
                break;
            }
            int port = atoi(cmd->payload);
            faro_fd = netty_start_server(port);
            if (faro_fd < 0) {
                ipc_log_error("FARO_BIND_FAILED");
            } else {
                ipc_send_response("FARO_OK", 0, "LISTENING");
            }
            break;

        case CMD_NEW_CHAT:
            slot = get_free_session_slot();
            if (slot == -1) {
                ipc_log_error("MAX_SESSIONS_REACHED");
                break;
            }

            char ip[64] = {0};
            int t_port = 0;
            sscanf(cmd->payload, "%[^:]:%d", ip, &t_port);

            int new_sock = netty_connect_to_peer(ip, t_port);
            if (new_sock < 0) {
                ipc_log_error("CONNECTION_FAILED");
                break;
            }

            pChat new_arena = newChat(cmd->chat_id);
            if (new_arena == NULL) {
                netty_close(new_sock);
                ipc_log_error("RAM_LOCK_FAILED");
                break;
            }
        
            sessions[slot].chat_id = cmd->chat_id;
            sessions[slot].sock_fd = new_sock;
            sessions[slot].arena = new_arena;
            sessions[slot].active = 1;
            sessions[slot].handshake_state = STATE_WAIT_PK;
        
            ipc_send_response("NEW_OK", cmd->chat_id, "CONNECTED_WAITING_KEYS");
            break;

        case CMD_SEND_MSG:
            s_idx = find_session_by_id(cmd->chat_id);
            if (s_idx != -1) {
                if (sessions[s_idx].handshake_state != STATE_READY) {
                    ipc_log_error("HANDSHAKE_IN_PROGRESS");
                    break;
                }

                newMessage(sessions[s_idx].arena, cmd->payload);
                
                unsigned char net_buffer[1024];
                size_t net_len = 0;
                
                if (encryptNetMessage(sessions[s_idx].arena, cmd->payload, net_buffer, &net_len) == 0) {
                    netty_send_data(sessions[s_idx].sock_fd, net_buffer, net_len);
                    ipc_send_response("SEND_OK", cmd->chat_id, "STORED_AND_SENT");
                }
            }
            break;  

        case CMD_GET_HISTORY:
            s_idx = find_session_by_id(cmd->chat_id);
            if (s_idx != -1) {
                current_query_chat_id = cmd->chat_id;
                getChatHistory(sessions[s_idx].arena, on_history_decrypted);
            }
            break;

        default:
            break;
    }
}

void engine_handle_net(int sock_fd, const unsigned char* data, size_t len) {
    if (data == NULL || len == 0) return;

    int s_idx = find_session_by_sock(sock_fd);
    if (s_idx == -1) return;

    if (sessions[s_idx].handshake_state == STATE_WAIT_PK) {
        if (len < crypto_kem_mlkem768_PUBLICKEYBYTES) return;
        
        unsigned char ciphertext[crypto_kem_mlkem768_CIPHERTEXTBYTES];
        unsigned char shared_secret[crypto_kem_mlkem768_SHAREDSECRETBYTES];

        if (crypto_kem_mlkem768_enc(ciphertext, shared_secret, data) == 0) {
            
            setSharedSecret(sessions[s_idx].arena, shared_secret);
            
            netty_send_data(sock_fd, ciphertext, sizeof(ciphertext));
            
            sodium_memzero(shared_secret, sizeof(shared_secret));
            sessions[s_idx].handshake_state = STATE_READY;
            
            ipc_send_response("SYS_MSG", sessions[s_idx].chat_id, "QUANTUM_HANDSHAKE_COMPLETE");
        } else {
            ipc_log_error("KEM_ENCAPSULATION_FAILED");
        }
        return;
    }

    if (sessions[s_idx].handshake_state == STATE_WAIT_CT) {
        if (len < crypto_kem_mlkem768_CIPHERTEXTBYTES) return;
        
        unsigned char shared_secret[crypto_kem_mlkem768_SHAREDSECRETBYTES];

        if (crypto_kem_mlkem768_dec(shared_secret, data, sessions[s_idx].sk) == 0) {
            
            setSharedSecret(sessions[s_idx].arena, shared_secret);
            
            sodium_memzero(sessions[s_idx].sk, sizeof(sessions[s_idx].sk));
            sodium_memzero(shared_secret, sizeof(shared_secret));
            
            sessions[s_idx].handshake_state = STATE_READY;
            ipc_send_response("SYS_MSG", sessions[s_idx].chat_id, "QUANTUM_HANDSHAKE_COMPLETE");
        } else {
            ipc_log_error("KEM_DECAPSULATION_FAILED");
        }
        return;
    }

    if (sessions[s_idx].handshake_state == STATE_READY) {
        char plain_text[512] = {0};
        
        if (decryptNetMessage(sessions[s_idx].arena, data, len, plain_text) == 0) {
            newMessage(sessions[s_idx].arena, plain_text);
            ipc_send_response("INC_MSG", sessions[s_idx].chat_id, plain_text);
        } else {
            ipc_log_error("DECRYPT_NETWORK_FAILED");
        }
    }
}

int engine_get_active_sockets(struct pollfd* fds, int max_fds) {
    int count = 0;
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (sessions[i].active && sessions[i].sock_fd != -1 && count < max_fds) {
            fds[count].fd = sessions[i].sock_fd;
            fds[count].events = POLLIN;
            fds[count].revents = 0;
            count++;
        }
    }
    return count;
}

void engine_remove_socket(int sock_fd) {
    for (int i = 0; i < MAX_SESSIONS; i++) {
        if (sessions[i].active && sessions[i].sock_fd == sock_fd) {
            sessions[i].sock_fd = -1;
            break;
        }
    }
}
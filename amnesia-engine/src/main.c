#include <stdio.h>
#include <unistd.h>
#include <poll.h>
#include <sodium.h>
#include <sys/resource.h>
#include <string.h>
#include <arpa/inet.h>

#include "engine.h"
#include "netty.h"
#include "parser.h"

int main(void) {
    struct rlimit core_limit;
    core_limit.rlim_cur = 0;
    core_limit.rlim_max = 0;

    // Desactivamos buffers para evitar copias ocultas en el Heap de glibc
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    if (setrlimit(RLIMIT_CORE, &core_limit) < 0) {
        fprintf(stderr, "WARNING: Can't deactivate core dumps!\n");
    }

    if (sodium_init() < 0) {
        fprintf(stderr, "FATAL: Can't initiate libsodium.\n");
        return 1;
    }

    engine_init();

    struct pollfd fds[11];

    while (1) {
        fds[0].fd = STDIN_FILENO;
        fds[0].events = POLLIN;
        fds[0].revents = 0;

        int nfds = 1;
        int current_faro = engine_get_faro_fd();
        if (current_faro >= 0) {
            fds[nfds].fd = current_faro;
            fds[nfds].events = POLLIN;
            fds[nfds].revents = 0;
            nfds++;
        }

        nfds += engine_get_active_sockets(&fds[nfds], 11 - nfds);

        int ret = poll(fds, nfds, -1);
        
        if (ret < 0) {
            continue; 
        }

        // Bloque corregido (sin duplicación)
        if (fds[0].revents & POLLIN) {
            ParsedCommand cmd;
            
            if (ipc_read_command(&cmd)) {
                engine_handle_ipc(&cmd);
                // Hardening: Borramos el comando de la pila inmediatamente
                sodium_memzero(&cmd, sizeof(ParsedCommand));
            } else {
                engine_panic();
                break;
            }
        }

        for (int i = 1; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {
                int current_faro = engine_get_faro_fd();
                
                if (current_faro >= 0 && fds[i].fd == current_faro) {
                    engine_accept_faro();
                } else {
                    unsigned char net_buffer[1024];
                    int bytes = netty_read_data(fds[i].fd, net_buffer, sizeof(net_buffer));
                    
                    if (bytes > 0) {
                        engine_handle_net(fds[i].fd, net_buffer, bytes);
                    } else {
                        netty_close(fds[i].fd);
                        engine_remove_socket(fds[i].fd);
                    }
                }
            }
        }
    }

    return 0;
}
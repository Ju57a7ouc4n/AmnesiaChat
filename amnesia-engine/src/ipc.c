#include "ipc.h"

int ipc_read_command(ParsedCommand* cmd) {
    char buffer[MAX_IPC_BUFFER];

    if (fgets(buffer, sizeof(buffer), stdin) == NULL) {
        return 0;
    }

    buffer[strcspn(buffer, "\n")] = '\0';

    cmd->type = CMD_UNKNOWN;
    cmd->chat_id = 0;
    memset(cmd->payload, 0, MAX_IPC_BUFFER);

    char* first_pipe = strchr(buffer, '|');
    if (first_pipe == NULL) return 1; 

    *first_pipe = '\0';
    char* cmd_str = buffer;
    char* remainder = first_pipe + 1;

    char* second_pipe = strchr(remainder, '|');
    if (second_pipe != NULL) {
        *second_pipe = '\0';
        cmd->chat_id = (uint32_t)strtoul(remainder, NULL, 10);
        strncpy(cmd->payload, second_pipe + 1, MAX_IPC_BUFFER - 1);
    } else {
        cmd->chat_id = (uint32_t)strtoul(remainder, NULL, 10);
    }

    if (strcmp(cmd_str, "NEW") == 0) cmd->type = CMD_NEW_CHAT;
    else if (strcmp(cmd_str, "SEND") == 0) cmd->type = CMD_SEND_MSG;
    else if (strcmp(cmd_str, "HIST") == 0) cmd->type = CMD_GET_HISTORY;
    else if (strcmp(cmd_str, "PANIC") == 0) cmd->type = CMD_PANIC;
    else if (strcmp(cmd_str, "FARO") == 0) cmd->type = CMD_START_FARO;
    
    return 1;
}

void ipc_send_response(const char* action, uint32_t chat_id, const char* data) {
    if (action == NULL || data == NULL) return;
    
    printf("%s|%u|%s\n", action, chat_id, data);
    fflush(stdout);
}

void ipc_log_error(const char* msg) {
    if (msg != NULL) {
        fprintf(stderr, "ERROR: %s\n", msg);
        fflush(stderr);
    }
}
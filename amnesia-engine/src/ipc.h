#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAX_IPC_BUFFER 1024

typedef enum {
    CMD_NEW_CHAT,
    CMD_SEND_MSG,
    CMD_GET_HISTORY,
    CMD_PANIC,
    CMD_UNKNOWN,
    CMD_START_FARO
} IpcCommandType;

typedef struct {
    IpcCommandType type;
    uint32_t chat_id;
    char payload[MAX_IPC_BUFFER];
} ParsedCommand;

int ipc_read_command(ParsedCommand* cmd);
void ipc_send_response(const char* action, uint32_t chat_id, const char* data);
void ipc_log_error(const char* msg);
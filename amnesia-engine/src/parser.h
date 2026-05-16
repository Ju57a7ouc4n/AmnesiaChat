#include <stdlib.h>
#include <string.h>

typedef struct {
    char cmd[16];
    int chat_id;
    char payload[1024];
} IPC_Command;

IPC_Command parse_java_string(char* raw_string);
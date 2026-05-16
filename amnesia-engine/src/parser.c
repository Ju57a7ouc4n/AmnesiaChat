#include "parser.h"
#include <string.h>

IPC_Command parse_java_string(char* raw_string) {
    IPC_Command req = {0};
    char* first_pipe = strchr(raw_string, '|');
    if (first_pipe == NULL) return req;
    
    *first_pipe = '\0';
    
    char* second_pipe = strchr(first_pipe + 1, '|');
    if (second_pipe == NULL) return req;
    
    *second_pipe = '\0';

    strncpy(req.cmd, raw_string, sizeof(req.cmd) - 1);
    
    req.chat_id = atoi(first_pipe + 1); 
    char* payload_start = second_pipe + 1;
    payload_start[strcspn(payload_start, "\r\n")] = 0; 
    
    strncpy(req.payload, payload_start, sizeof(req.payload) - 1);
    
    return req;
}

#define ENGINE_H

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>
#include "ipc.h"
#include "vault.h"
#include "netty.h"

#define MAX_SESSIONS 10

typedef struct {
    uint32_t chat_id;
    int sock_fd;
    pChat arena;
    int active;
} Session;

void engine_init(void);
void engine_handle_ipc(ParsedCommand* cmd);
void engine_handle_net(int sock_fd, const unsigned char* data, size_t len);
void engine_panic(void);
void engine_remove_socket(int sock_fd);
int engine_get_active_sockets(struct pollfd* fds, int max_fds);
int engine_get_faro_fd(void);
void engine_accept_faro(void);
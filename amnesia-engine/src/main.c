#include <unistd.h>
#include <poll.h>
#include <sodium.h>
#include "engine.h"
#include "netty.h"

int main(void) {
    if (sodium_init() < 0) {
        return 1;
    }

    engine_init();

    struct pollfd fds[11];
    
    while (1) {
        fds[0].fd = STDIN_FILENO;
        fds[0].events = POLLIN;
        fds[0].revents = 0;

        int nfds = 1;
        nfds += engine_get_active_sockets(&fds[1], 10);

        int ret = poll(fds, nfds, -1);
        
        if (ret < 0) {
            continue;
        }

        if (fds[0].revents & POLLIN) {
            ParsedCommand cmd;
            if (ipc_read_command(&cmd)) {
                engine_handle_ipc(&cmd);
            } else {
                engine_panic();
            }
        }

        for (int i = 1; i < nfds; i++) {
            if (fds[i].revents & POLLIN) {
                unsigned char buffer[1024];
                int bytes = netty_read_data(fds[i].fd, buffer, sizeof(buffer));
                
                if (bytes > 0) {
                    engine_handle_net(fds[i].fd, buffer, bytes);
                } else {
                    netty_close(fds[i].fd);
                    engine_remove_socket(fds[i].fd);
                }
            }
        }
    }

    return 0;
}
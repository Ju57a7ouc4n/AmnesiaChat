#define NETTY_H

#include <stddef.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

int netty_start_server(int port);
int netty_accept_client(int server_fd, char* client_ip);
int netty_connect_to_peer(const char* ip, int port);
int netty_send_data(int sock_fd, const unsigned char* data, size_t len);
int netty_read_data(int sock_fd, unsigned char* buffer, size_t max_len);
void netty_close(int sock_fd);

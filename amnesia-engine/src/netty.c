#include "netty.h"

int netty_start_server(int port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) return -1;

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        close(server_fd);
        return -1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, 10) < 0) {
        close(server_fd);
        return -1;
    }

    return server_fd;
}

int netty_accept_client(int server_fd, char* client_ip) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);

    if (client_fd >= 0 && client_ip != NULL) {
        inet_ntop(AF_INET, &client_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
    }

    return client_fd;
}

int netty_connect_to_peer(const char* ip, int port) {
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) return -1;

    struct sockaddr_in peer_addr;
    memset(&peer_addr, 0, sizeof(peer_addr));
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &peer_addr.sin_addr) <= 0) {
        close(sock_fd);
        return -1;
    }

    if (connect(sock_fd, (struct sockaddr*)&peer_addr, sizeof(peer_addr)) < 0) {
        close(sock_fd);
        return -1;
    }

    return sock_fd;
}

int netty_send_data(int sock_fd, const unsigned char* data, size_t len) {
    if (sock_fd < 0 || data == NULL || len == 0) return -1;
    return send(sock_fd, data, len, 0);
}

int netty_read_data(int sock_fd, unsigned char* buffer, size_t max_len) {
    if (sock_fd < 0 || buffer == NULL || max_len == 0) return -1;
    return recv(sock_fd, buffer, max_len, 0);
}

void netty_close(int sock_fd) {
    if (sock_fd >= 0) {
        close(sock_fd);
    }
}
/*
 * client_net.c - TCP networking implementation
 *
 * Opens a single persistent TCP socket to the chat server.
 * All control messages (register, login, group ops) go through here.
 */

#include "client_net.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

/* The single TCP socket; -1 means not connected */
static int g_sockfd = -1;

/*
 * connect_to_server - Create TCP socket and connect to ip:port.
 * Returns the socket fd on success, -1 on failure.
 */
int connect_to_server(const char *ip, int port)
{
    struct sockaddr_in server_addr;

    g_sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (g_sockfd < 0) {
        perror("[Net] socket");
        return -1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons((unsigned short)port);

    if (inet_pton(AF_INET, ip, &server_addr.sin_addr) <= 0) {
        fprintf(stderr, "[Net] Invalid address: %s\n", ip);
        close(g_sockfd);
        g_sockfd = -1;
        return -1;
    }

    if (connect(g_sockfd, (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0) {
        perror("[Net] connect");
        close(g_sockfd);
        g_sockfd = -1;
        return -1;
    }

    printf("[Net] Connected to %s:%d\n", ip, port);
    return g_sockfd;
}

/*
 * send_request - Send exactly 'len' bytes from 'data'.
 * Returns bytes sent on success, -1 on error.
 */
int send_request(const void *data, size_t len)
{
    if (g_sockfd < 0) {
        fprintf(stderr, "[Net] Not connected\n");
        return -1;
    }

    ssize_t sent = send(g_sockfd, data, len, 0);
    if (sent < 0) {
        perror("[Net] send");
        return -1;
    }
    return (int)sent;
}

/*
 * receive_response - Read up to 'maxlen' bytes into 'buffer'.
 * Returns bytes received, 0 on server disconnect, -1 on error.
 */
int receive_response(void *buffer, size_t maxlen)
{
    if (g_sockfd < 0) {
        fprintf(stderr, "[Net] Not connected\n");
        return -1;
    }

    ssize_t received = recv(g_sockfd, buffer, maxlen, 0);
    if (received < 0) {
        perror("[Net] recv");
        return -1;
    }
    if (received == 0) {
        printf("[Net] Server closed the connection\n");
    }
    return (int)received;
}

/*
 * close_connection - Shutdown and close the TCP socket.
 */
void close_connection(void)
{
    if (g_sockfd >= 0) {
        shutdown(g_sockfd, SHUT_RDWR);
        close(g_sockfd);
        g_sockfd = -1;
        printf("[Net] Connection closed\n");
    }
}

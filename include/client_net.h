#ifndef CLIENT_NET_H
#define CLIENT_NET_H

/*
 * client_net.h - TCP networking interface
 *
 * Manages a single persistent TCP connection to the chat server.
 */

#include <stddef.h>

/* Connect to the chat server.
 * Returns socket fd on success, -1 on failure. */
int connect_to_server(const char *ip, int port);

/* Send exactly 'len' bytes to the server.
 * Returns bytes sent on success, -1 on error. */
int send_request(const void *data, size_t len);

/* Receive up to 'maxlen' bytes from the server.
 * Returns bytes received, 0 on disconnect, -1 on error. */
int receive_response(void *buffer, size_t maxlen);

/* Close the TCP connection gracefully. */
void close_connection(void);

#endif /* CLIENT_NET_H */

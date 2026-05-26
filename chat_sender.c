#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// This program sends chat messages to a TCP server.
// Usage: chat_sender <server_ip> <port> <username>
// Example: chat_sender 127.0.0.1 12345 Alice

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Usage: %s <server_ip> <port> <username>\n", argv[0]);
        return 1;
    }
    // Parse command line arguments
    char* server_ip = argv[1];
    int port = atoi(argv[2]);
    char* username = argv[3];

    // Create TCP socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    // Set up server address
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(server_ip);
    serv_addr.sin_port = htons(port);

    // Connect to server
    if (connect(sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect");
        exit(1);
    }

    // Send username to server
    send(sock, username, strlen(username), 0);

    char msg[512];
    while (1) {
        // Prompt user for message
        printf("[%s] Enter message: ", username);
        fgets(msg, sizeof(msg), stdin);
        // Send message to server
        send(sock, msg, strlen(msg), 0);
    }
    // Close socket (unreachable code in this loop)
    close(sock);
    return 0;
}

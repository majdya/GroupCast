#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// This program receives chat messages from a TCP server and prints them to the screen.
// Usage: chat_receiver <server_ip> <port>
// Example: chat_receiver 127.0.0.1 12345

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <server_ip> <port>\n", argv[0]);
        return 1;
    }
    // Parse command line arguments
    char* server_ip = argv[1];
    int port = atoi(argv[2]);

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

    char msg[600];
    while (1) {
        // Receive message from server
        int n = recv(sock, msg, sizeof(msg)-1, 0);
        if (n <= 0) break;
        msg[n] = '\0';
        // Print received message
        printf("%s", msg);
        fflush(stdout);
    }
    // Close socket (if loop is broken)
    close(sock);
    return 0;
}
        printf("%s", msg);

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// This program sends chat messages to a multicast group.
// Usage: chat_sender <multicast_ip> <port> <username>
// Example: chat_sender 239.0.0.1 12345 Alice

int main(int argc, char* argv[]) {
    if (argc != 4) {
        printf("Usage: %s <multicast_ip> <port> <username>\n", argv[0]);
        return 1;
    }
    // Parse command line arguments
    char* multicast_ip = argv[1];
    int port = atoi(argv[2]);
    char* username = argv[3];

    // Create UDP socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    // Set up destination address (multicast group)
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(multicast_ip);
    addr.sin_port = htons(port);

    char msg[512];
    while (1) {
        // Prompt user for message
        printf("[%s] Enter message: ", username);
        fgets(msg, sizeof(msg), stdin);
        // Format message as "username: message"
        char sendbuf[600];
        snprintf(sendbuf, sizeof(sendbuf), "%s: %s", username, msg);
        // Send message to multicast group
        sendto(sock, sendbuf, strlen(sendbuf), 0, (struct sockaddr*)&addr, sizeof(addr));
    }
    // Close socket (unreachable code in this loop)
    close(sock);
    return 0;
}

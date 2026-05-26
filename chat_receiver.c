#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

// This program receives chat messages from a multicast group and prints them to the screen.
// Usage: chat_receiver <multicast_ip> <port>
// Example: chat_receiver 239.0.0.1 12345

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <multicast_ip> <port>\n", argv[0]);
        return 1;
    }
    // Parse command line arguments
    char* multicast_ip = argv[1];
    int port = atoi(argv[2]);  /// atoi  = text to number 

    // Create TCP socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        exit(1);
    }

    // Set up local address to bind to
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port);

    // Bind socket to the specified port
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(1);
    }

    // Join the multicast group
    struct ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = inet_addr(multicast_ip);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq)) < 0) {
        perror("setsockopt");
        exit(1);
    }

    char msg[600];
    while (1) {
        // Receive message from multicast group
        int n = recv(sock, msg, sizeof(msg)-1, 0);
        if (n < 0) {
            perror("recv");
            break;
        }
        msg[n] = '\0';
        // Print received message
        printf("%s", msg);
        fflush(stdout);
    }
    // Close socket (if loop is broken)
    close(sock);
    return 0;
}

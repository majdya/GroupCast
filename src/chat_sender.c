/*
 * chat_sender.c - Standalone multicast sender
 *
 * This is a separate executable launched by the main client in a
 * gnome-terminal window. It:
 *   1. Sends its PID back to the client via message queue
 *   2. Reads messages from stdin
 *   3. Sends each message to the multicast group
 *   4. Exits gracefully on SIGTERM or empty input
 *
 * Usage: ./chat_sender <multicast_ip> <port> <mq_id>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define MAX_MSG_LEN 512

/* Message queue struct (must match common.h) */
typedef struct {
    long  mtype;
    pid_t pid;
} MqPidMsg;

#define MQ_TYPE_SENDER 2

/* Flag for clean shutdown */
static volatile int g_running = 1;

/* SIGTERM handler */
static void sigterm_handler(int sig)
{
    (void)sig;
    g_running = 0;
}

int main(int argc, char *argv[])
{
    int                sockfd;
    struct sockaddr_in dest_addr;
    char               buf[MAX_MSG_LEN + 1];

    /* Parse command line arguments */
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <multicast_ip> <port> <mq_id>\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *multicast_ip = argv[1];
    int port   = atoi(argv[2]);
    int mq_id  = atoi(argv[3]);

    /* Install SIGTERM handler for graceful shutdown */
    struct sigaction sa;
    sa.sa_handler = sigterm_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGTERM, &sa, NULL);

    /* Send our PID back to the main client via message queue */
    MqPidMsg msg;
    msg.mtype = MQ_TYPE_SENDER;
    msg.pid   = getpid();
    if (msgsnd(mq_id, &msg, sizeof(msg.pid), 0) < 0) {
        perror("[Sender] msgsnd");
        /* Continue anyway */
    }

    /* Create UDP socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("[Sender] socket");
        return EXIT_FAILURE;
    }

    /* Set multicast TTL to 1 (local network only) */
    unsigned char ttl = 1;
    if (setsockopt(sockfd, IPPROTO_IP, IP_MULTICAST_TTL,
                   &ttl, sizeof(ttl)) < 0) {
        perror("[Sender] IP_MULTICAST_TTL");
        close(sockfd);
        return EXIT_FAILURE;
    }

    /* Set destination address */
    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port   = htons((unsigned short)port);

    if (inet_pton(AF_INET, multicast_ip, &dest_addr.sin_addr) <= 0) {
        fprintf(stderr, "[Sender] Invalid multicast address: %s\n",
                multicast_ip);
        close(sockfd);
        return EXIT_FAILURE;
    }

    printf("=== Type messages to send to %s:%d ===\n", multicast_ip, port);
    printf("(This window will close when you leave the group)\n\n");

    /* Send loop: read from stdin, send to multicast */
    while (g_running) {
        printf("> ");
        fflush(stdout);

        if (!fgets(buf, sizeof(buf), stdin)) {
            break;  /* EOF */
        }

        /* Strip trailing newline */
        size_t len = strlen(buf);
        if (len > 0 && buf[len - 1] == '\n') {
            buf[--len] = '\0';
        }

        /* Empty line = do nothing (keep running until killed) */
        if (len == 0) {
            continue;
        }

        /* Send the message to multicast */
        ssize_t sent = sendto(sockfd, buf, len, 0,
                              (struct sockaddr *)&dest_addr,
                              sizeof(dest_addr));
        if (sent < 0) {
            perror("[Sender] sendto");
        }
    }

    close(sockfd);
    printf("\n[Sender] Stopped.\n");
    return EXIT_SUCCESS;
}

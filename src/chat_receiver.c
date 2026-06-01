/*
 * chat_receiver.c - Standalone multicast receiver
 *
 * This is a separate executable launched by the main client in a
 * gnome-terminal window. It:
 *   1. Sends its PID back to the client via message queue
 *   2. Joins the multicast group
 *   3. Prints every message it receives
 *   4. Exits gracefully on SIGTERM
 *
 * Usage: ./chat_receiver <multicast_ip> <port> <mq_id>
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
#include <time.h>

#define MAX_MSG_LEN 512

/* Message queue struct (must match common.h) */
typedef struct {
    long  mtype;
    pid_t pid;
} MqPidMsg;

#define MQ_TYPE_RECEIVER 1

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
    struct sockaddr_in local_addr;
    struct ip_mreq     mcast_req;
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
    msg.mtype = MQ_TYPE_RECEIVER;
    msg.pid   = getpid();
    if (msgsnd(mq_id, &msg, sizeof(msg.pid), 0) < 0) {
        perror("[Receiver] msgsnd");
        /* Continue anyway - the receiver can still work */
    }

    /* Create UDP socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("[Receiver] socket");
        return EXIT_FAILURE;
    }

    /* Allow multiple sockets on same port */
    int reuse = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

    /* Bind to the multicast port */
    memset(&local_addr, 0, sizeof(local_addr));
    local_addr.sin_family      = AF_INET;
    local_addr.sin_port        = htons((unsigned short)port);
    local_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&local_addr,
             sizeof(local_addr)) < 0) {
        perror("[Receiver] bind");
        close(sockfd);
        return EXIT_FAILURE;
    }

    /* Join the multicast group */
    memset(&mcast_req, 0, sizeof(mcast_req));
    mcast_req.imr_interface.s_addr = htonl(INADDR_ANY);
    if (inet_pton(AF_INET, multicast_ip, &mcast_req.imr_multiaddr) <= 0) {
        fprintf(stderr, "[Receiver] Invalid multicast address: %s\n",
                multicast_ip);
        close(sockfd);
        return EXIT_FAILURE;
    }

    if (setsockopt(sockfd, IPPROTO_IP, IP_ADD_MEMBERSHIP,
                   &mcast_req, sizeof(mcast_req)) < 0) {
        perror("[Receiver] IP_ADD_MEMBERSHIP");
        close(sockfd);
        return EXIT_FAILURE;
    }

    printf("=== Receiving messages on %s:%d ===\n", multicast_ip, port);
    printf("(This window will close when you leave the group)\n\n");

    /* Receive loop */
    while (g_running) {
        struct sockaddr_in sender_addr;
        socklen_t addr_len = sizeof(sender_addr);

        ssize_t n = recvfrom(sockfd, buf, MAX_MSG_LEN, 0,
                             (struct sockaddr *)&sender_addr, &addr_len);
        if (n < 0) {
            if (!g_running) break;  /* interrupted by signal */
            perror("[Receiver] recvfrom");
            break;
        }
        buf[n] = '\0';

        time_t now = time(NULL);
        char ts[9];
        strftime(ts, sizeof(ts), "%H:%M:%S", localtime(&now));
        printf("[%s] %s\n", ts, buf);
        fflush(stdout);
    }

    /* Leave multicast group and close */
    setsockopt(sockfd, IPPROTO_IP, IP_DROP_MEMBERSHIP,
               &mcast_req, sizeof(mcast_req));
    close(sockfd);
    printf("\n[Receiver] Stopped.\n");
    return EXIT_SUCCESS;
}

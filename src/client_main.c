/*
 * client_main.c - Main entry point for the Chat Client
 *
 * Flow:
 *   1. Connect to server via TCP
 *   2. Screen 1 loop: register / login / exit
 *   3. Screen 2 loop: create group / join group / leave group / logout
 *   4. On join/create success: open gnome-terminal windows for chat
 *   5. On leave: kill chat windows for that group
 *   6. On logout: kill all chat windows, return to Screen 1
 *   7. On exit: close connection, free memory
 */

#include "common.h"
#include "client_net.h"
#include "client_mng.h"
#include "client_groups_mng.h"
#include "ui.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>

/* Server connection settings */
#define SERVER_IP   "127.0.0.1"
#define SERVER_PORT 8888

/* Timeout for reading PID from message queue (seconds) */
#define MQ_TIMEOUT_SEC 5

/* ----------------------------------------------------------------
 * build_and_send - Pack the right payload for a MessageType and send.
 * Returns 0 on success, -1 on failure.
 * ---------------------------------------------------------------- */
static int build_and_send(MessageType type, const char *username,
                          const char *password, const char *group_name)
{
    TlvPacket pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.type = (uint8_t)type;

    switch (type) {
        case REGISTER_REQ:
        case LOGIN_REQ:
            /* Payload: packed username + password */
            pkt.length = (uint8_t)Proto_PackUserPass(
                pkt.payload,
                username ? username : "",
                password ? password : "");
            break;

        case CREATE_GROUP_REQ:
        case JOIN_GROUP_REQ:
        case LEAVE_GROUP_REQ:
            /* Payload: packed group name */
            pkt.length = (uint8_t)Proto_PackStr(
                pkt.payload,
                group_name ? group_name : "");
            break;

        case LOGOUT_REQ:
            /* Payload: packed username */
            pkt.length = (uint8_t)Proto_PackStr(
                pkt.payload,
                username ? username : "");
            break;

        default:
            pkt.length = 0;
            break;
    }

    /* Wire size = type(1) + length(1) + payload */
    size_t wire_len = 2 + pkt.length;
    return send_request(&pkt, wire_len) > 0 ? 0 : -1;
}

/* ----------------------------------------------------------------
 * recv_and_handle - Read one TlvPacket from server and dispatch it.
 * Returns 0 on success, -1 on error/disconnect.
 * ---------------------------------------------------------------- */
static int recv_and_handle(void)
{
    uint8_t header[2];
    if (receive_response(header, 2) <= 0) return -1;

    TlvPacket pkt;
    pkt.type   = header[0];
    pkt.length = header[1];

    if (pkt.length > 0) {
        if (receive_response(pkt.payload, pkt.length) <= 0) return -1;
    }

    handle_server_response(&pkt);
    return 0;
}

/* ----------------------------------------------------------------
 * create_mq - Create a System V message queue.
 * Returns the mq id, or -1 on failure.
 * ---------------------------------------------------------------- */
static int create_mq(void)
{
    /* Use IPC_PRIVATE to get a unique queue for this client */
    int mq_id = msgget(IPC_PRIVATE, 0666 | IPC_CREAT);
    if (mq_id < 0) {
        perror("[Client] msgget");
    }
    return mq_id;
}

/* ----------------------------------------------------------------
 * destroy_mq - Remove a message queue.
 * ---------------------------------------------------------------- */
static void destroy_mq(int mq_id)
{
    if (mq_id >= 0) {
        msgctl(mq_id, IPC_RMID, NULL);
    }
}

/* ----------------------------------------------------------------
 * read_pid_from_mq - Wait for a PID message from the queue.
 * mtype: MQ_TYPE_RECEIVER or MQ_TYPE_SENDER
 * Returns the PID on success, -1 on failure/timeout.
 * ---------------------------------------------------------------- */
static pid_t read_pid_from_mq(int mq_id, long mtype)
{
    MqPidMsg msg;
    /* Use blocking receive with alarm for timeout */
    alarm(MQ_TIMEOUT_SEC);
    ssize_t ret = msgrcv(mq_id, &msg, sizeof(msg.pid), mtype, 0);
    alarm(0);

    if (ret < 0) {
        perror("[Client] msgrcv");
        return -1;
    }
    return msg.pid;
}

/* ----------------------------------------------------------------
 * launch_chat_windows - Open receiver and sender in gnome-terminal.
 * Stores PIDs in the GroupInfo struct.
 * Returns 0 on success, -1 on failure.
 * ---------------------------------------------------------------- */
static int launch_chat_windows(const char *group_name,
                               const char *multicast_ip, int port,
                               const char *username,
                               GroupInfo *out_group)
{
    char cmd[512];
    int mq_id;
    pid_t recv_pid, send_pid;

    /* Create message queue for PID communication */
    mq_id = create_mq();
    if (mq_id < 0) return -1;

    /* Launch receiver in a new terminal window */
    snprintf(cmd, sizeof(cmd),
             "gnome-terminal --title=\"[%s] Receiver\" -- "
             "./chat_receiver %s %d %d &",
             group_name, multicast_ip, port, mq_id);
    system(cmd);

    /* Launch sender in a new terminal window */
    snprintf(cmd, sizeof(cmd),
             "gnome-terminal --title=\"[%s] Sender\" -- "
             "./chat_sender %s %d %d %s &",
             group_name, multicast_ip, port, mq_id,
             username ? username : "");
    system(cmd);

    /* Read PIDs from message queue */
    recv_pid = read_pid_from_mq(mq_id, MQ_TYPE_RECEIVER);
    send_pid = read_pid_from_mq(mq_id, MQ_TYPE_SENDER);

    /* Clean up message queue (no longer needed) */
    destroy_mq(mq_id);

    if (recv_pid < 0 || send_pid < 0) {
        fprintf(stderr, "[Client] Failed to get PIDs from chat windows\n");
        return -1;
    }

    /* Fill out the GroupInfo */
    memset(out_group, 0, sizeof(GroupInfo));
    strncpy(out_group->name, group_name, MAX_GROUP_NAME_LEN - 1);
    strncpy(out_group->multicast_ip, multicast_ip, MAX_IP_LEN - 1);
    out_group->port         = port;
    out_group->receiver_pid = recv_pid;
    out_group->sender_pid   = send_pid;

    printf("[Client] Chat windows opened for '%s' "
           "(receiver PID=%d, sender PID=%d)\n",
           group_name, recv_pid, send_pid);
    return 0;
}

/* ----------------------------------------------------------------
 * SIGALRM handler (used for msgrcv timeout)
 * ---------------------------------------------------------------- */
static void sigalrm_handler(int sig)
{
    (void)sig;
    /* Do nothing - just interrupt msgrcv */
}

/* ----------------------------------------------------------------
 * main
 * ---------------------------------------------------------------- */
int main(int argc, char* argv[])
{
    uint16_t port = SERVER_PORT;
    if (argc > 1) {
        long p = atol(argv[1]);
        if (p > 0 && p <= 65535) port = (uint16_t)p;
    }

    char username[MAX_USERNAME_LEN]    = {0};
    char password[MAX_PASSWORD_LEN]    = {0};
    char group_name[MAX_GROUP_NAME_LEN] = {0};

    /* Install SIGALRM handler for message queue timeout */
    struct sigaction sa;
    sa.sa_handler = sigalrm_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGALRM, &sa, NULL);

    printf("[Client] Starting Chat Client...\n");

    /* 1. Connect to server */
    if (connect_to_server(SERVER_IP, port) < 0) {
        fprintf(stderr, "[Client] Cannot connect to %s:%d\n",
                SERVER_IP, port);
        return EXIT_FAILURE;
    }

    /* Outer loop: allows returning to Screen 1 after logout */
    while (1) {

        /* ---- Screen 1: Register / Login / Exit ---- */
        int logged_in = 0;
        while (!logged_in) {
            int action = show_login_screen(username, password);

            if (action == UI_EXIT) {
                printf("[Client] Goodbye!\n");
                close_connection();
                return EXIT_SUCCESS;
            }

            /* Map UI action to wire message type */
            MessageType req = (action == UI_LOGIN) ? LOGIN_REQ : REGISTER_REQ;

            if (build_and_send(req, username, password, NULL) < 0) {
                fprintf(stderr, "[Client] Send failed, try again.\n");
                continue;
            }
            if (recv_and_handle() < 0) {
                fprintf(stderr, "[Client] Server disconnected.\n");
                close_connection();
                return EXIT_FAILURE;
            }

            /* If login was successful, move to Screen 2 */
            if (action == UI_LOGIN && last_login_ok()) {
                logged_in = 1;
            }
            /* Registration: stay on Screen 1 so user can now login */
        }

        /* ---- Screen 2: Create / Join / Leave / Logout ---- */
        int stay_in_groups = 1;
        while (stay_in_groups) {
            int action = show_group_screen(group_name);

            switch (action) {

                case UI_CREATE_GROUP:
                case UI_JOIN_GROUP: {
                    /* Send create or join request */
                    MessageType req = (action == UI_CREATE_GROUP)
                                      ? CREATE_GROUP_REQ : JOIN_GROUP_REQ;

                    if (build_and_send(req, username, NULL, group_name) < 0) {
                        fprintf(stderr, "[Client] Send failed.\n");
                        break;
                    }

                    pending_group_clear();
                    if (recv_and_handle() < 0) {
                        fprintf(stderr, "[Client] Server disconnected.\n");
                        close_connection();
                        groups_cleanup();
                        return EXIT_FAILURE;
                    }

                    /* If server approved, launch chat windows */
                    char pending_ip[MAX_IP_LEN];
                    int  pending_port;
                    if (pending_group_get(pending_ip, &pending_port)) {
                        GroupInfo g;
                        if (launch_chat_windows(group_name, pending_ip,
                                               pending_port, username,
                                               &g) == 0) {
                            add_group(&g);
                        }
                    }
                    break;
                }

                case UI_LEAVE_GROUP: {
                    if (build_and_send(LEAVE_GROUP_REQ, username,
                                      NULL, group_name) < 0) {
                        fprintf(stderr, "[Client] Send failed.\n");
                        break;
                    }
                    if (recv_and_handle() < 0) {
                        fprintf(stderr, "[Client] Server disconnected.\n");
                        close_connection();
                        groups_cleanup();
                        return EXIT_FAILURE;
                    }
                    /* remove_group also kills the chat windows */
                    remove_group(group_name);
                    break;
                }

                case UI_LOGOUT: {
                    /* Send logout to server */
                    build_and_send(LOGOUT_REQ, username, NULL, NULL);
                    recv_and_handle();

                    /* Kill all chat windows and free groups */
                    groups_kill_all();
                    groups_cleanup();

                    /* Return to Screen 1 */
                    stay_in_groups = 0;
                    break;
                }

                default:
                    break;
            }
        }
    }

    close_connection();
    groups_cleanup();
    return EXIT_SUCCESS;
}

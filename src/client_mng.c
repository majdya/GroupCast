/*
 * client_mng.c - Server response handler implementation
 *
 * Receives a TlvPacket, unpacks the payload using Proto_Unpack*
 * helpers from protocol.h, and prints the result.
 */

#include "client_mng.h"
#include <stdio.h>
#include <string.h>

/* ---- Private handler prototypes ---- */
static void handle_register_resp(const TlvPacket *pkt);
static void handle_login_resp(const TlvPacket *pkt);
static void handle_create_group_resp(const TlvPacket *pkt);
static void handle_join_resp(const TlvPacket *pkt);
static void handle_leave_resp(const TlvPacket *pkt);
static void handle_logout_resp(const TlvPacket *pkt);

/* ---- Pending group scratch pad ---- */
/* JOIN/CREATE success stores multicast info here for main to read */
static char g_pending_ip[MAX_IP_LEN] = {0};
static int  g_pending_port           = 0;
static int  g_pending_valid          = 0;

/* ---- Last login status ---- */
static int g_login_ok = 0;

void pending_group_clear(void)
{
    g_pending_ip[0] = '\0';
    g_pending_port  = 0;
    g_pending_valid = 0;
}

int pending_group_get(char *ip_out, int *port_out)
{
    if (!g_pending_valid) {
        return 0;
    }
    strncpy(ip_out, g_pending_ip, MAX_IP_LEN - 1);
    ip_out[MAX_IP_LEN - 1] = '\0';
    *port_out = g_pending_port;
    return 1;
}

int last_login_ok(void)
{
    return g_login_ok;
}

/* ---- Main dispatcher ---- */
void handle_server_response(const TlvPacket *pkt)
{
    if (!pkt) {
        fprintf(stderr, "[Mng] NULL packet\n");
        return;
    }

    switch (pkt->type) {
        case REGISTER_RESP:     handle_register_resp(pkt);      break;
        case LOGIN_RESP:        handle_login_resp(pkt);         break;
        case CREATE_GROUP_RESP: handle_create_group_resp(pkt);  break;
        case JOIN_GROUP_RESP:   handle_join_resp(pkt);          break;
        case LEAVE_GROUP_RESP:  handle_leave_resp(pkt);         break;
        case LOGOUT_RESP:       handle_logout_resp(pkt);        break;
        default:
            fprintf(stderr, "[Mng] Unknown message type: 0x%02X\n",
                    pkt->type);
            break;
    }
}

/* ---- Private handlers ---- */

static void handle_register_resp(const TlvPacket *pkt)
{
    uint8_t status = pkt->payload[0];
    if (status == SUCCESS) {
        printf("[Server] Registration successful.\n");
    } else {
        printf("[Server] Registration failed: %s\n",
               Proto_StatusStr(status));
    }
}

static void handle_login_resp(const TlvPacket *pkt)
{
    uint8_t status = pkt->payload[0];
    g_login_ok = (status == SUCCESS);
    if (status == SUCCESS) {
        printf("[Server] Login successful.\n");
    } else {
        printf("[Server] Login failed: %s\n", Proto_StatusStr(status));
    }
}

static void handle_create_group_resp(const TlvPacket *pkt)
{
    /* On success, the response carries multicast IP + port (same as join) */
    char     addr[MAX_IP_LEN] = {0};
    uint16_t port = 0;
    uint8_t  status;

    if (Proto_UnpackGroupResp(pkt->payload, pkt->length,
                              &status, addr, sizeof(addr), &port) != 0) {
        fprintf(stderr, "[Mng] Failed to unpack CREATE response\n");
        return;
    }

    if (status == SUCCESS) {
        /* Store pending info - client_main will start receiver/sender */
        strncpy(g_pending_ip, addr, MAX_IP_LEN - 1);
        g_pending_port  = port;
        g_pending_valid = 1;
        printf("[Server] Group created - multicast %s:%d\n", addr, port);
    } else {
        printf("[Server] Create group failed: %s\n",
               Proto_StatusStr(status));
    }
}

static void handle_join_resp(const TlvPacket *pkt)
{
    char     addr[MAX_IP_LEN] = {0};
    uint16_t port = 0;
    uint8_t  status;

    if (Proto_UnpackGroupResp(pkt->payload, pkt->length,
                              &status, addr, sizeof(addr), &port) != 0) {
        fprintf(stderr, "[Mng] Failed to unpack JOIN response\n");
        return;
    }

    if (status == SUCCESS) {
        /* Store pending info - client_main will start receiver/sender */
        strncpy(g_pending_ip, addr, MAX_IP_LEN - 1);
        g_pending_port  = port;
        g_pending_valid = 1;
        printf("[Server] Joined group - multicast %s:%d\n", addr, port);
    } else {
        printf("[Server] Join failed: %s\n", Proto_StatusStr(status));
    }
}

static void handle_leave_resp(const TlvPacket *pkt)
{
    uint8_t status = pkt->payload[0];
    if (status == SUCCESS) {
        printf("[Server] Left group successfully.\n");
    } else {
        printf("[Server] Leave failed: %s\n", Proto_StatusStr(status));
    }
}

static void handle_logout_resp(const TlvPacket *pkt)
{
    uint8_t status = pkt->payload[0];
    if (status == SUCCESS) {
        printf("[Server] Logged out successfully.\n");
    } else {
        printf("[Server] Logout error: %s\n", Proto_StatusStr(status));
    }
}

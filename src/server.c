#include "server.h"
#include "server_mng.h"
#include "group_mng.h"
#include "types.h"
#include "protocol.h"
#include "comm_link.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/select.h>

#define MAX_CLIENTS 64

typedef struct {
    CommPeer* peer;
    char      username[USER_NAME_MAX + 1];
    int       in_use;
} Session;

struct Server {
    ServerMng* m_mng;
    GroupMng*  m_gmng;
    CommPeer*  m_listener;
    Session    m_sessions[MAX_CLIENTS];
    int        m_running;
};

Server* Server_Create(uint16_t port)
{
    Server* srv = (Server*)calloc(1, sizeof(Server));
    if (!srv) return NULL;

    srv->m_mng = ServerMng_Create();
    if (!srv->m_mng) { free(srv); return NULL; }

    srv->m_gmng = GroupMng_Create();
    if (!srv->m_gmng) { ServerMng_Destroy(&srv->m_mng); free(srv); return NULL; }

    srv->m_listener = Comm_Listen(port);
    if (!srv->m_listener) {
        GroupMng_Destroy(&srv->m_gmng);
        ServerMng_Destroy(&srv->m_mng);
        free(srv);
        return NULL;
    }

    srv->m_running = 1;
    return srv;
}

void Server_Destroy(Server** srv)
{
    if (!srv || !*srv) return;
    Server* s = *srv;
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (s->m_sessions[i].in_use)
            Comm_Close(s->m_sessions[i].peer);
    }
    Comm_Close(s->m_listener);
    GroupMng_Destroy(&s->m_gmng);
    ServerMng_Destroy(&s->m_mng);
    free(s);
    *srv = NULL;
}

static int find_free_slot(Session* sessions)
{
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (!sessions[i].in_use) return i;
    }
    return -1;
}

static void handle_message(Server* srv, Session* session, uint8_t type, uint8_t* buf, uint8_t len)
{
    switch (type) {
        case REGISTER_REQ: {
            char user[USER_NAME_MAX + 1];
            char pass[PASSWORD_MAX + 1];
            if (Proto_UnpackUserPass(buf, len, user, sizeof(user), pass, sizeof(pass)) != 0) {
                uint8_t resp = ERR_GENERAL;
                Comm_Send(session->peer, REGISTER_RESP, &resp, 1);
                return;
            }
            int status = ServerMng_Register(srv->m_mng, user, pass);
            uint8_t resp = (uint8_t)status;
            Comm_Send(session->peer, REGISTER_RESP, &resp, 1);
            break;
        }

        case LOGIN_REQ: {
            char user[USER_NAME_MAX + 1];
            char pass[PASSWORD_MAX + 1];
            if (Proto_UnpackUserPass(buf, len, user, sizeof(user), pass, sizeof(pass)) != 0) {
                uint8_t resp = ERR_GENERAL;
                Comm_Send(session->peer, LOGIN_RESP, &resp, 1);
                return;
            }
            int status = ServerMng_Login(srv->m_mng, user, pass);
            if (status == SUCCESS)
                strcpy(session->username, user);
            uint8_t resp = (uint8_t)status;
            Comm_Send(session->peer, LOGIN_RESP, &resp, 1);
            break;
        }

        case LOGOUT_REQ: {
            session->username[0] = '\0';
            uint8_t resp = SUCCESS;
            Comm_Send(session->peer, LOGOUT_RESP, &resp, 1);
            break;
        }

        case CREATE_GROUP_REQ: {
            uint8_t rbuf[TLV_MAX_PAYLOAD];
            uint8_t rlen;

            if (session->username[0] == '\0') {
                rlen = (uint8_t)Proto_PackGroupResp(rbuf, ERR_GENERAL, NULL, 0);
                Comm_Send(session->peer, CREATE_GROUP_RESP, rbuf, rlen);
                return;
            }
            char gname[GROUP_NAME_MAX + 1];
            if (Proto_UnpackStr(buf, len, gname, sizeof(gname)) != 0) {
                rlen = (uint8_t)Proto_PackGroupResp(rbuf, ERR_GENERAL, NULL, 0);
                Comm_Send(session->peer, CREATE_GROUP_RESP, rbuf, rlen);
                return;
            }
            char addr[16];
            uint16_t port;
            int status = GroupMng_GroupCreate(srv->m_gmng, gname, addr, &port);
            rlen = (uint8_t)Proto_PackGroupResp(rbuf, (uint8_t)status, addr, port);
            Comm_Send(session->peer, CREATE_GROUP_RESP, rbuf, rlen);
            break;
        }

        case JOIN_GROUP_REQ: {
            uint8_t rbuf[TLV_MAX_PAYLOAD];
            uint8_t rlen;

            if (session->username[0] == '\0') {
                rlen = (uint8_t)Proto_PackGroupResp(rbuf, ERR_GENERAL, NULL, 0);
                Comm_Send(session->peer, JOIN_GROUP_RESP, rbuf, rlen);
                return;
            }
            char gname[GROUP_NAME_MAX + 1];
            if (Proto_UnpackStr(buf, len, gname, sizeof(gname)) != 0) {
                rlen = (uint8_t)Proto_PackGroupResp(rbuf, ERR_GENERAL, NULL, 0);
                Comm_Send(session->peer, JOIN_GROUP_RESP, rbuf, rlen);
                return;
            }
            char addr[16];
            uint16_t port;
            int status = GroupMng_Join(srv->m_gmng, gname, session->username, addr, &port);
            rlen = (uint8_t)Proto_PackGroupResp(rbuf, (uint8_t)status, addr, port);
            Comm_Send(session->peer, JOIN_GROUP_RESP, rbuf, rlen);
            break;
        }

        case LEAVE_GROUP_REQ: {
            uint8_t rbuf[TLV_MAX_PAYLOAD];
            uint8_t rlen;

            if (session->username[0] == '\0') {
                rlen = Proto_PackGroupResp(rbuf, ERR_GENERAL, NULL, 0);
                Comm_Send(session->peer, LEAVE_GROUP_RESP, rbuf, rlen);
                return;
            }
            char gname[GROUP_NAME_MAX + 1];
            if (Proto_UnpackStr(buf, len, gname, sizeof(gname)) != 0) {
                uint8_t resp = ERR_GENERAL;
                Comm_Send(session->peer, LEAVE_GROUP_RESP, &resp, 1);
                return;
            }
            int status = GroupMng_Leave(srv->m_gmng, gname, session->username);
            uint8_t resp = (uint8_t)status;
            Comm_Send(session->peer, LEAVE_GROUP_RESP, &resp, 1);
            break;
        }
    }
}

void Server_Run(Server* srv)
{
    if (!srv) return;

    while (srv->m_running) {
        fd_set rfds;
        FD_ZERO(&rfds);

        int max_fd = Comm_FD(srv->m_listener);
        FD_SET(max_fd, &rfds);

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (!srv->m_sessions[i].in_use) continue;
            int fd = Comm_FD(srv->m_sessions[i].peer);
            FD_SET(fd, &rfds);
            if (fd > max_fd) max_fd = fd;
        }

        int n = select(max_fd + 1, &rfds, NULL, NULL, NULL);
        if (n < 0) {
            if (errno == EINTR) continue;
            break;
        }

        if (FD_ISSET(Comm_FD(srv->m_listener), &rfds)) {
            CommPeer* peer = Comm_Accept(srv->m_listener);
            if (peer) {
                int slot = find_free_slot(srv->m_sessions);
                if (slot >= 0) {
                    srv->m_sessions[slot].peer = peer;
                    srv->m_sessions[slot].username[0] = '\0';
                    srv->m_sessions[slot].in_use = 1;
                } else {
                    Comm_Close(peer);
                }
            }
        }

        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (!srv->m_sessions[i].in_use) continue;
            int fd = Comm_FD(srv->m_sessions[i].peer);
            if (!FD_ISSET(fd, &rfds)) continue;

            uint8_t type, buf[TLV_MAX_PAYLOAD], len;
            CommResult res = Comm_TryRecv(srv->m_sessions[i].peer, &type, buf, &len);
            if (res == COMM_MSG_READY) {
                handle_message(srv, &srv->m_sessions[i], type, buf, len);
            } else if (res == COMM_CLOSED || res == COMM_ERROR) {
                Comm_Close(srv->m_sessions[i].peer);
                srv->m_sessions[i].in_use = 0;
                srv->m_sessions[i].username[0] = '\0';
            }
        }
    }
}



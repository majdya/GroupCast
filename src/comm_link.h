#ifndef __COMM_LINK_H__
#define __COMM_LINK_H__

#include <stdint.h>
#include <stddef.h>

typedef struct CommPeer CommPeer;

typedef enum {
    COMM_MSG_READY = 1,
    COMM_AGAIN     = 0,
    COMM_CLOSED    = -1,
    COMM_ERROR     = -2
} CommResult;

CommPeer* Comm_Listen(uint16_t port);
CommPeer* Comm_Accept(CommPeer* server);
CommPeer* Comm_Connect(const char* addr, uint16_t port);
void      Comm_Close(CommPeer* peer);

int       Comm_Send(CommPeer* peer, uint8_t type, const void* data, uint8_t len);
CommResult Comm_TryRecv(CommPeer* peer, uint8_t* outType, void* outBuf, uint8_t* outLen);
int       Comm_FD(CommPeer* peer);

CommPeer* Comm_FromFd(int fd);

#endif /* __COMM_LINK_H__ */

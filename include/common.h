#ifndef COMMON_H
#define COMMON_H

/*
 * common.h - Client-side shared definitions
 *
 * Includes the wire protocol and adds client-specific types:
 *   - TlvPacket struct
 *   - Buffer size constants
 *   - Message queue struct for PID communication
 */

#include "protocol.h"
#include <sys/types.h>

/* ---- Buffer size constants ---- */
#define MAX_USERNAME_LEN   32
#define MAX_PASSWORD_LEN   32
#define MAX_GROUP_NAME_LEN 64
#define MAX_MSG_LEN        512
#define MAX_IP_LEN         16

/* ---- TLV Packet: the wire format for every message ---- */
typedef struct {
    uint8_t type;                    /* MessageType value */
    uint8_t length;                  /* number of payload bytes (0-255) */
    uint8_t payload[TLV_MAX_PAYLOAD];
} TlvPacket;

/* ---- Message queue struct for PID communication ---- */
/* Used by chat_receiver/chat_sender to send their PID back to the client */
typedef struct {
    long mtype;       /* must be > 0; we use 1 for receiver, 2 for sender */
    pid_t pid;        /* the PID of the child process */
} MqPidMsg;

#define MQ_TYPE_RECEIVER 1
#define MQ_TYPE_SENDER   2

#endif /* COMMON_H */

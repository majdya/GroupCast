#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include <stdint.h>
#include <stddef.h>

#define TLV_MAX_PAYLOAD 255

typedef enum {
    REGISTER_REQ      = 0x01,
    REGISTER_RESP     = 0x02,
    LOGIN_REQ         = 0x03,
    LOGIN_RESP        = 0x04,
    CREATE_GROUP_REQ  = 0x05,
    CREATE_GROUP_RESP = 0x06,
    JOIN_GROUP_REQ    = 0x07,
    JOIN_GROUP_RESP   = 0x08,
    LEAVE_GROUP_REQ   = 0x09,
    LEAVE_GROUP_RESP  = 0x0A,
    LOGOUT_REQ        = 0x0B,
    LOGOUT_RESP       = 0x0C
} MessageType;

typedef enum {
    SUCCESS              = 0,
    ERR_USER_EXISTS      = 1,
    ERR_USER_NOT_FOUND   = 2,
    ERR_WRONG_PASSWORD   = 3,
    ERR_GROUP_EXISTS     = 4,
    ERR_GROUP_NOT_FOUND  = 5,
    ERR_ALREADY_IN_GROUP = 6,
    ERR_NOT_IN_GROUP     = 7,
    ERR_GENERAL          = 8
} StatusCode;

size_t Proto_PackStr(uint8_t* buf, const char* str);
int    Proto_UnpackStr(const uint8_t* buf, size_t len, char* out, size_t outCap);

size_t Proto_PackUserPass(uint8_t* buf, const char* user, const char* pass);
int    Proto_UnpackUserPass(const uint8_t* buf, size_t len, char* user, size_t userCap, char* pass, size_t passCap);

size_t Proto_PackGroupResp(uint8_t* buf, uint8_t status, const char* addr, uint16_t port);
int    Proto_UnpackGroupResp(const uint8_t* buf, size_t len, uint8_t* status, char* addr, size_t addrCap, uint16_t* port);

const char* Proto_StatusStr(uint8_t status);

#endif /* __PROTOCOL_H__ */

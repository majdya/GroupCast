#include "protocol.h"
#include <string.h>
#include <arpa/inet.h>

size_t Proto_PackStr(uint8_t* buf, const char* str)
{
    size_t len = str ? strlen(str) : 0;
    if (len > TLV_MAX_PAYLOAD - 1) len = TLV_MAX_PAYLOAD - 1;
    buf[0] = (uint8_t)len;
    if (len > 0) memcpy(buf + 1, str, len);
    return 1 + len;
}

int Proto_UnpackStr(const uint8_t* buf, size_t len, char* out, size_t outCap)
{
    if (len < 1) return -1;
    size_t slen = buf[0];
    if (1 + slen > len) return -1;
    size_t copy = slen < outCap - 1 ? slen : outCap - 1;
    memcpy(out, buf + 1, copy);
    out[copy] = '\0';
    return 0;
}

size_t Proto_PackUserPass(uint8_t* buf, const char* user, const char* pass)
{
    size_t off = Proto_PackStr(buf, user);
    off += Proto_PackStr(buf + off, pass);
    return off;
}

int Proto_UnpackUserPass(const uint8_t* buf, size_t len, char* user, size_t userCap, char* pass, size_t passCap)
{
    size_t off = 0;
    if (len < 1) return -1;
    size_t ulen = buf[0];
    if (1 + ulen > len) return -1;
    size_t ucpy = ulen < userCap - 1 ? ulen : userCap - 1;
    memcpy(user, buf + 1, ucpy);
    user[ucpy] = '\0';
    off = 1 + ulen;
    if (off >= len) return -1;
    size_t plen = buf[off];
    if (off + 1 + plen > len) return -1;
    size_t pcpy = plen < passCap - 1 ? plen : passCap - 1;
    memcpy(pass, buf + off + 1, pcpy);
    pass[pcpy] = '\0';
    return 0;
}

size_t Proto_PackGroupResp(uint8_t* buf, uint8_t status, const char* addr, uint16_t port)
{
    buf[0] = status;
    if (status != SUCCESS) return 1;
    size_t off = 1;
    off += Proto_PackStr(buf + off, addr);
    if (off + 2 > TLV_MAX_PAYLOAD) return 1;
    uint16_t nport = htons(port);
    memcpy(buf + off, &nport, 2);
    return off + 2;
}

int Proto_UnpackGroupResp(const uint8_t* buf, size_t len, uint8_t* status, char* addr, size_t addrCap, uint16_t* port)
{
    if (len < 1) return -1;
    *status = buf[0];
    if (*status != SUCCESS) return 0;
    if (len < 2) return -1;
    size_t off = 1;
    if (Proto_UnpackStr(buf + off, len - off, addr, addrCap) != 0) return -1;
    off += 1 + (size_t)buf[off];
    if (off + 2 > len) return -1;
    uint16_t nport;
    memcpy(&nport, buf + off, 2);
    *port = ntohs(nport);
    return 0;
}

const char* Proto_StatusStr(uint8_t status)
{
    switch (status) {
        case SUCCESS:              return "SUCCESS";
        case ERR_USER_EXISTS:      return "ERR_USER_EXISTS";
        case ERR_USER_NOT_FOUND:   return "ERR_USER_NOT_FOUND";
        case ERR_WRONG_PASSWORD:   return "ERR_WRONG_PASSWORD";
        case ERR_GROUP_EXISTS:     return "ERR_GROUP_EXISTS";
        case ERR_GROUP_NOT_FOUND:  return "ERR_GROUP_NOT_FOUND";
        case ERR_ALREADY_IN_GROUP: return "ERR_ALREADY_IN_GROUP";
        case ERR_NOT_IN_GROUP:     return "ERR_NOT_IN_GROUP";
        case ERR_GENERAL:          return "ERR_GENERAL";
        default:                   return "UNKNOWN";
    }
}

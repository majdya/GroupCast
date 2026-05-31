#ifndef __TYPES_H__
#define __TYPES_H__

#include <stdint.h>

#define USER_NAME_MAX  31
#define PASSWORD_MAX   31
#define GROUP_NAME_MAX 31

typedef struct {
    char     m_name[GROUP_NAME_MAX + 1];
    char     m_addr[16];
    uint16_t m_port;
    uint32_t m_user_count;
} GroupInfo;

#endif /* __TYPES_H__ */

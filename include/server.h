#ifndef __SERVER_H__
#define __SERVER_H__

#include <stdint.h>

typedef struct Server Server;

Server* Server_Create(uint16_t port);
void    Server_Destroy(Server** server);
void    Server_Run(Server* server);

#endif /* __SERVER_H__ */

#ifndef __SERVER_MNG_H__
#define __SERVER_MNG_H__

typedef struct ServerMng ServerMng;

ServerMng* ServerMng_Create(void);
void ServerMng_Destroy(ServerMng** mng);

int ServerMng_Register(ServerMng* mng, const char* user, const char* pass);
int ServerMng_Login(ServerMng* mng, const char* user, const char* pass);

#endif /* __SERVER_MNG_H__ */

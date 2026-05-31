#ifndef __GROUP_MNG_H__
#define __GROUP_MNG_H__

#include <stdint.h>

typedef struct GroupMng GroupMng;

GroupMng* GroupMng_Create(void);
void GroupMng_Destroy(GroupMng** mng);

int GroupMng_GroupCreate(GroupMng* mng, const char* name, char* outAddr, uint16_t* outPort);
int GroupMng_Join(GroupMng* mng, const char* name, const char* user, char* outAddr, uint16_t* outPort);
int GroupMng_Leave(GroupMng* mng, const char* name, const char* user);

#endif /* __GROUP_MNG_H__ */

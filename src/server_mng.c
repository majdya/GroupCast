#include "server_mng.h"
#include "types.h"
#include "protocol.h"
#include "hash_map.h"
#include <stdlib.h>
#include <string.h>

struct ServerMng {
    HashMap* m_users;
};

static size_t hash_str(void* key)
{
    size_t h = 0;
    unsigned char* p = (unsigned char*)key;
    while (*p) { h = h * 31 + *p++; }
    return h;
}

static int eq_str(void* a, void* b)
{
    return strcmp((const char*)a, (const char*)b) == 0;
}

ServerMng* ServerMng_Create(void)
{
    ServerMng* mng = (ServerMng*)calloc(1, sizeof(ServerMng));
    if (!mng) return NULL;
    mng->m_users = HashMap_Create(64, hash_str, eq_str);
    if (!mng->m_users) {
        free(mng);
        return NULL;
    }
    return mng;
}

void ServerMng_Destroy(ServerMng** mng)
{
    if (!mng || !*mng) return;
    HashMap_Destroy(&(*mng)->m_users, free, free);
    free(*mng);
    *mng = NULL;
}

int ServerMng_Register(ServerMng* mng, const char* user, const char* pass)
{
    if (!mng || !user || !pass) return ERR_GENERAL;
    if (strlen(user) > USER_NAME_MAX || strlen(pass) > PASSWORD_MAX)
        return ERR_GENERAL;

    void* existing;
    if (HashMap_Find(mng->m_users, user, &existing) == MAP_SUCCESS)
        return ERR_USER_EXISTS;

    char* key = strdup(user);
    char* val = strdup(pass);
    if (!key || !val) {
        free(key);
        free(val);
        return ERR_GENERAL;
    }

    Map_Result res = HashMap_Insert(mng->m_users, key, val);
    if (res != MAP_SUCCESS) {
        free(key);
        free(val);
        return ERR_GENERAL;
    }
    return SUCCESS;
}

int ServerMng_Login(ServerMng* mng, const char* user, const char* pass)
{
    if (!mng || !user || !pass) return ERR_GENERAL;

    void* stored_pass;
    if (HashMap_Find(mng->m_users, user, &stored_pass) != MAP_SUCCESS)
        return ERR_USER_NOT_FOUND;

    if (strcmp((const char*)stored_pass, pass) != 0)
        return ERR_WRONG_PASSWORD;

    return SUCCESS;
}

#include "group_mng.h"
#include "types.h"
#include "protocol.h"
#include "hash_map.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define MC_BASE       "239.255.0."
#define PORT_BASE 50000
#define MAX_GROUPS 64

typedef struct {
    char     m_addr[16];
    uint16_t m_port;
    int      m_mc_idx;
    HashMap* m_members;
} GroupData;

struct GroupMng {
    HashMap* m_groups;
    uint64_t m_mc_bitmap;
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

GroupMng* GroupMng_Create(void)
{
    GroupMng* mng = (GroupMng*)calloc(1, sizeof(GroupMng));
    if (!mng) return NULL;
    mng->m_groups = HashMap_Create(32, hash_str, eq_str);
    if (!mng->m_groups) {
        free(mng);
        return NULL;
    }
    mng->m_mc_bitmap = 0;
    return mng;
}

static void destroy_group_data(void* data)
{
    GroupData* gd = (GroupData*)data;
    if (!gd) return;
    HashMap_Destroy(&gd->m_members, free, NULL);
    free(gd);
}

void GroupMng_Destroy(GroupMng** mng)
{
    if (!mng || !*mng) return;
    HashMap_Destroy(&(*mng)->m_groups, free, destroy_group_data);
    free(*mng);
    *mng = NULL;
}

static int alloc_mc(GroupMng* mng)
{
    for (int i = 1; i < MAX_GROUPS; i++) {
        if (!(mng->m_mc_bitmap & (1ULL << i))) {
            mng->m_mc_bitmap |= (1ULL << i);
            return i;
        }
    }
    return 0;
}

static void free_mc(GroupMng* mng, int idx)
{
    mng->m_mc_bitmap &= ~(1ULL << idx);
}

int GroupMng_GroupCreate(GroupMng* mng, const char* name, char* outAddr, uint16_t* outPort)
{
    if (!mng || !name || !outAddr || !outPort) return ERR_GENERAL;
    if (strlen(name) > GROUP_NAME_MAX) return ERR_GENERAL;

    void* existing;
    if (HashMap_Find(mng->m_groups, name, &existing) == MAP_SUCCESS)
        return ERR_GROUP_EXISTS;

    int idx = alloc_mc(mng);
    if (idx == 0) return ERR_GENERAL;

    snprintf(outAddr, 16, "%s%d", MC_BASE, idx);
    *outPort = PORT_BASE + idx;

    GroupData* gd = (GroupData*)calloc(1, sizeof(GroupData));
    if (!gd) { free_mc(mng, idx); return ERR_GENERAL; }

    snprintf(gd->m_addr, 16, "%s%d", MC_BASE, idx);
    gd->m_port = PORT_BASE + idx;
    gd->m_mc_idx = idx;
    gd->m_members = HashMap_Create(16, hash_str, eq_str);
    if (!gd->m_members) {
        free(gd);
        free_mc(mng, idx);
        return ERR_GENERAL;
    }

    char* key = strdup(name);
    if (!key) {
        HashMap_Destroy(&gd->m_members, NULL, NULL);
        free(gd);
        free_mc(mng, idx);
        return ERR_GENERAL;
    }

    Map_Result res = HashMap_Insert(mng->m_groups, key, gd);
    if (res != MAP_SUCCESS) {
        free(key);
        HashMap_Destroy(&gd->m_members, NULL, NULL);
        free(gd);
        free_mc(mng, idx);
        return ERR_GENERAL;
    }

    return SUCCESS;
}

int GroupMng_Join(GroupMng* mng, const char* name, const char* user, char* outAddr, uint16_t* outPort)
{
    if (!mng || !name || !user || !outAddr || !outPort) return ERR_GENERAL;

    void* gd_ptr;
    if (HashMap_Find(mng->m_groups, name, &gd_ptr) != MAP_SUCCESS)
        return ERR_GROUP_NOT_FOUND;

    GroupData* gd = (GroupData*)gd_ptr;

    if (HashMap_Find(gd->m_members, user, NULL) == MAP_SUCCESS)
        return ERR_ALREADY_IN_GROUP;

    char* member_key = strdup(user);
    if (!member_key) return ERR_GENERAL;

    Map_Result res = HashMap_Insert(gd->m_members, member_key, NULL);
    if (res != MAP_SUCCESS) {
        free(member_key);
        return ERR_GENERAL;
    }

    strcpy(outAddr, gd->m_addr);
    *outPort = gd->m_port;
    return SUCCESS;
}

int GroupMng_Leave(GroupMng* mng, const char* name, const char* user)
{
    if (!mng || !name || !user) return ERR_GENERAL;

    void* gd_ptr;
    if (HashMap_Find(mng->m_groups, name, &gd_ptr) != MAP_SUCCESS)
        return ERR_GROUP_NOT_FOUND;

    GroupData* gd = (GroupData*)gd_ptr;

    void* removed_key;
    Map_Result res = HashMap_Remove(gd->m_members, user, &removed_key, NULL);
    if (res != MAP_SUCCESS)
        return ERR_NOT_IN_GROUP;

    free(removed_key);

    if (HashMap_Size(gd->m_members) == 0) {
        void* group_key;
        HashMap_Remove(mng->m_groups, name, &group_key, NULL);
        free(group_key);

        if (gd->m_mc_idx > 0) free_mc(mng, gd->m_mc_idx);

        HashMap_Destroy(&gd->m_members, free, NULL);
        free(gd);
    }

    return SUCCESS;
}

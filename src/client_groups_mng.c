/*
 * client_groups_mng.c - Local group list management implementation
 *
 * Uses gen_dlist (doubly-linked list) to store GroupInfo entries.
 * Handles killing receiver/sender processes when removing groups.
 */

#include "client_groups_mng.h"
#include "gen_dlist.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>

/* The group list; created on first use */
static List *g_groups = NULL;

/* Ensure the list exists */
static void ensure_list(void)
{
    if (!g_groups) {
        g_groups = ListCreate();
    }
}

/* Kill receiver and sender processes for a group */
static void kill_group_processes(GroupInfo *g)
{
    if (g->receiver_pid > 0) {
        kill(g->receiver_pid, SIGTERM);
        g->receiver_pid = 0;
    }
    if (g->sender_pid > 0) {
        kill(g->sender_pid, SIGTERM);
        g->sender_pid = 0;
    }
}

/* ForEach callback context for finding a group by name */
typedef struct {
    const char *target;
    GroupInfo  *found;
} FindCtx;

/* ForEach callback: return 0 to stop when found */
static int find_action(void *element, void *context)
{
    GroupInfo *g = (GroupInfo *)element;
    FindCtx *ctx = (FindCtx *)context;

    if (strncmp(g->name, ctx->target, MAX_GROUP_NAME_LEN) == 0) {
        ctx->found = g;
        return 0;  /* stop iteration */
    }
    return 1;  /* continue */
}

/*
 * add_group - Add or update a group in the list.
 */
void add_group(const GroupInfo *group)
{
    if (!group) return;
    ensure_list();

    /* If already exists, update it */
    GroupInfo *existing = get_group(group->name);
    if (existing) {
        memcpy(existing, group, sizeof(GroupInfo));
        return;
    }

    /* Allocate a copy and push to tail */
    GroupInfo *copy = (GroupInfo *)malloc(sizeof(GroupInfo));
    if (!copy) {
        fprintf(stderr, "[Groups] Memory allocation failed\n");
        return;
    }
    memcpy(copy, group, sizeof(GroupInfo));
    ListPushTail(g_groups, copy);
    printf("[Groups] Joined '%s' (multicast %s:%d)\n",
           copy->name, copy->multicast_ip, copy->port);
}

/*
 * remove_group - Find by name, kill processes, remove from list, free.
 */
void remove_group(const char *group_name)
{
    if (!group_name || !g_groups) return;

    ListItr itr = ListItrBegin(g_groups);
    ListItr end = ListItrEnd(g_groups);

    while (itr != end) {
        GroupInfo *g = (GroupInfo *)ListItrGet(itr);
        if (g && strncmp(g->name, group_name, MAX_GROUP_NAME_LEN) == 0) {
            kill_group_processes(g);
            ListItrRemove(itr);
            free(g);
            printf("[Groups] Removed '%s'\n", group_name);
            return;
        }
        itr = ListItrNext(itr);
    }

    printf("[Groups] Group '%s' not found\n", group_name);
}

/*
 * get_group - Find a group by name. Returns NULL if not found.
 */
GroupInfo *get_group(const char *group_name)
{
    if (!group_name || !g_groups) return NULL;

    FindCtx ctx = {group_name, NULL};
    ListItrForEach(ListItrBegin(g_groups), ListItrEnd(g_groups),
                   find_action, &ctx);
    return ctx.found;
}

/*
 * list_groups - Print all groups to stdout.
 */
static int print_action(void *element, void *context)
{
    (void)context;
    GroupInfo *g = (GroupInfo *)element;
    printf("  %-20s  %s:%d\n", g->name, g->multicast_ip, g->port);
    return 1;
}

void list_groups(void)
{
    if (!g_groups || ListIsEmpty(g_groups)) {
        printf("[Groups] No groups joined.\n");
        return;
    }
    printf("[Groups] Joined groups:\n");
    printf("  %-20s  %s\n", "Name", "Multicast");
    printf("  %-20s  %s\n", "----", "---------");
    ListItrForEach(ListItrBegin(g_groups), ListItrEnd(g_groups),
                   print_action, NULL);
}

/*
 * groups_kill_all - Kill all receiver/sender processes for all groups.
 * Used on logout to close all chat windows.
 */
static int kill_all_action(void *element, void *context)
{
    (void)context;
    GroupInfo *g = (GroupInfo *)element;
    kill_group_processes(g);
    return 1;
}

void groups_kill_all(void)
{
    if (!g_groups) return;
    ListItrForEach(ListItrBegin(g_groups), ListItrEnd(g_groups),
                   kill_all_action, NULL);
}

/*
 * groups_cleanup - Destroy the list and free all GroupInfo memory.
 */
void groups_cleanup(void)
{
    if (g_groups) {
        /* Kill any remaining processes first */
        groups_kill_all();
        ListDestroy(&g_groups, free);
    }
}

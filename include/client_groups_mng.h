#ifndef CLIENT_GROUPS_MNG_H
#define CLIENT_GROUPS_MNG_H

/*
 * client_groups_mng.h - Local group list management
 *
 * Keeps track of joined groups using a doubly-linked list (gen_dlist).
 * Each group stores its name, multicast address, port, and the PIDs
 * of its receiver and sender processes.
 */

#include "common.h"
#include <sys/types.h>

/* Information about one joined group */
typedef struct {
    char    name[MAX_GROUP_NAME_LEN];
    char    multicast_ip[MAX_IP_LEN];
    int     port;
    pid_t   receiver_pid;   /* PID of gnome-terminal running chat_receiver */
    pid_t   sender_pid;     /* PID of gnome-terminal running chat_sender */
} GroupInfo;

/* Add a group to the list (copies the struct). Updates if name exists. */
void add_group(const GroupInfo *group);

/* Remove a group by name. Kills receiver/sender PIDs and frees memory. */
void remove_group(const char *group_name);

/* Find a group by name. Returns pointer to GroupInfo, or NULL. */
GroupInfo *get_group(const char *group_name);

/* Print all known groups to stdout. */
void list_groups(void);

/*
 * Kill all group processes (receiver + sender) and free all memory.
 * Call on logout to close all chat windows.
 */
void groups_kill_all(void);

/* Free all stored group data. Call once before exit. */
void groups_cleanup(void);

#endif /* CLIENT_GROUPS_MNG_H */

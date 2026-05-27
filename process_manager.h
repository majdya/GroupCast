#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

// Maximum number of groups a user can join simultaneously
#define MAX_GROUPS 20

// Structure to hold process information for each group
typedef struct {
    char group_name[64];      // Name of the group
    int sender_pid;           // PID of the sender process
    int receiver_pid;         // PID of the receiver process
} GroupProcess;

// Add a new group process entry
void add_group_process(const char* group_name, int sender_pid, int receiver_pid);
// Remove a group process entry by group name
void remove_group_process(const char* group_name);
// Close (terminate) all group processes
void close_all_group_processes();
// Close (terminate) a specific group process by group name
void close_group_process(const char* group_name);

#endif // PROCESS_MANAGER_H

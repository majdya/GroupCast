#include "process_manager.h"
#include <string.h>
#include <stdio.h>
#include <signal.h>

// Array to store process information for all joined groups
static GroupProcess group_processes[MAX_GROUPS];
// Number of currently joined groups
static int group_count = 0;

// Add a new group process entry to the array
void add_group_process(const char* group_name, int sender_pid, int receiver_pid) {
    if (group_count >= MAX_GROUPS) return;
    strncpy(group_processes[group_count].group_name, group_name, 63);
    group_processes[group_count].group_name[63] = '\0';
    group_processes[group_count].sender_pid = sender_pid;
    group_processes[group_count].receiver_pid = receiver_pid;
    group_count++;
}

// Remove a group process entry by group name
void remove_group_process(const char* group_name) {
    for (int i = 0; i < group_count; ++i) {
        if (strcmp(group_processes[i].group_name, group_name) == 0) {
            // Shift all entries left to fill the gap
            for (int j = i; j < group_count - 1; ++j) {
                group_processes[j] = group_processes[j+1];
            }
            group_count--;
            break;
        }
    }
}

// Close (terminate) both sender and receiver processes for a specific group
void close_group_process(const char* group_name) {
    for (int i = 0; i < group_count; ++i) {
        if (strcmp(group_processes[i].group_name, group_name) == 0) {
            kill(group_processes[i].sender_pid, SIGTERM);   // Terminate sender
            kill(group_processes[i].receiver_pid, SIGTERM); // Terminate receiver
            remove_group_process(group_name);
            break;
        }
    }
}

// Close (terminate) all sender and receiver processes for all groups
void close_all_group_processes() {
    for (int i = 0; i < group_count; ++i) {
        kill(group_processes[i].sender_pid, SIGTERM);
        kill(group_processes[i].receiver_pid, SIGTERM);
    }
    group_count = 0;
}

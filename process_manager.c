#include "process_manager.h"
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>


// Hash table size (prime number for better distribution)
#define HASH_TABLE_SIZE 101

// Node structure for chaining in hash table
typedef struct GroupProcessNode {
    GroupProcess data;
    struct GroupProcessNode* next;
} GroupProcessNode;

// Hash table: array of pointers to linked lists
static GroupProcessNode* hash_table[HASH_TABLE_SIZE] = {0};

// Hash function for group name (djb2)
static unsigned int hash_group_name(const char* group_name) {
    unsigned int hash = 5381;
    int c;
    while ((c = *group_name++)) {
        hash = ((hash << 5) + hash) + c; // hash * 33 + c
    }
    return hash % HASH_TABLE_SIZE;
}

// Add a new group process entry to the hash table
// Returns error if group already exists or on allocation failure
void add_group_process(const char* group_name, int sender_pid, int receiver_pid) {
    if (!group_name) {
        fprintf(stderr, "[ERROR] Group name is NULL.\n");
        return;
    }
    unsigned int idx = hash_group_name(group_name);
    GroupProcessNode* curr = hash_table[idx];
    // Check for duplicate group name
    while (curr) {
        if (strcmp(curr->data.group_name, group_name) == 0) {
            fprintf(stderr, "[ERROR] Group '%s' already exists.\n", group_name);
            return;
        }
        curr = curr->next;
    }
    // Allocate new node
    GroupProcessNode* new_node = (GroupProcessNode*)malloc(sizeof(GroupProcessNode));
    if (!new_node) {
        fprintf(stderr, "[ERROR] Memory allocation failed for group '%s'.\n", group_name);
        return;
    }
    strncpy(new_node->data.group_name, group_name, 63);
    new_node->data.group_name[63] = '\0';
    new_node->data.sender_pid = sender_pid;
    new_node->data.receiver_pid = receiver_pid;
    new_node->next = hash_table[idx];
    hash_table[idx] = new_node;
    printf("[INFO] Group '%s' added successfully.\n", group_name);
}

// Remove a group process entry by group name from the hash table
void remove_group_process(const char* group_name) {
    if (!group_name) {
        fprintf(stderr, "[ERROR] Group name is NULL.\n");
        return;
    }
    unsigned int idx = hash_group_name(group_name);
    GroupProcessNode* curr = hash_table[idx];
    GroupProcessNode* prev = NULL;
    while (curr) {
        if (strcmp(curr->data.group_name, group_name) == 0) {
            if (prev) {
                prev->next = curr->next;
            } else {
                hash_table[idx] = curr->next;
            }
            free(curr);
            printf("[INFO] Group '%s' removed successfully.\n", group_name);
            return;
        }
        prev = curr;
        curr = curr->next;
    }
    fprintf(stderr, "[ERROR] Group '%s' not found for removal.\n", group_name);
}

// Close (terminate) both sender and receiver processes for a specific group
void close_group_process(const char* group_name) {
    if (!group_name) {
        fprintf(stderr, "[ERROR] Group name is NULL.\n");
        return;
    }
    unsigned int idx = hash_group_name(group_name);
    GroupProcessNode* curr = hash_table[idx];
    while (curr) {
        if (strcmp(curr->data.group_name, group_name) == 0) {
            if (kill(curr->data.sender_pid, SIGTERM) == -1)
                perror("[ERROR] Failed to terminate sender process");
            if (kill(curr->data.receiver_pid, SIGTERM) == -1)
                perror("[ERROR] Failed to terminate receiver process");
            remove_group_process(group_name);
            printf("[INFO] Group '%s' processes terminated and removed.\n", group_name);
            return;
        }
        curr = curr->next;
    }
    fprintf(stderr, "[ERROR] Group '%s' not found for termination.\n", group_name);
}

// Close (terminate) all sender and receiver processes for all groups
void close_all_group_processes() {
    int found = 0;
    for (int i = 0; i < HASH_TABLE_SIZE; ++i) {
        GroupProcessNode* curr = hash_table[i];
        while (curr) {
            found = 1;
            if (kill(curr->data.sender_pid, SIGTERM) == -1)
                perror("[ERROR] Failed to terminate sender process");
            if (kill(curr->data.receiver_pid, SIGTERM) == -1)
                perror("[ERROR] Failed to terminate receiver process");
            curr = curr->next;
        }
    }
    // Free all nodes
    for (int i = 0; i < HASH_TABLE_SIZE; ++i) {
        GroupProcessNode* curr = hash_table[i];
        while (curr) {
            GroupProcessNode* tmp = curr;
            curr = curr->next;
            free(tmp);
        }
        hash_table[i] = NULL;
    }
    if (found)
        printf("[INFO] All group processes terminated and cleared.\n");
    else
        printf("[INFO] No groups to terminate.\n");
}

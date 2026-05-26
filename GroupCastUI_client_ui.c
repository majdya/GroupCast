#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function declarations
void show_main_menu();
void handle_registration();
void handle_login();
void handle_exit();
void show_group_menu(const char* username);
void handle_create_group(const char* username);
void handle_join_group(const char* username);
void handle_leave_group(const char* username);
void handle_logout(const char* username);
void open_chat_windows(const char* group_name, const char* username);

int main() {
    show_main_menu();
    return 0;
}

void show_main_menu() {
    int choice;
    while (1) {
        printf("=== GroupCast ===\n");
        printf("1. Registration\n");
        printf("2. Login\n");
        printf("3. Exit\n");
        printf("Choose option: ");
        scanf("%d", &choice);
        getchar(); // consume newline

        switch (choice) {
            case 1:
                handle_registration();
                break;
            case 2:
                handle_login();
                break;
            case 3:
                handle_exit();
                return;
            default:
                printf("Invalid option. Try again.\n");
        }
    }
}

void handle_registration() {
    char username[64], password[64];
    printf("Enter username: ");
    fgets(username, sizeof(username), stdin);
    printf("Enter password: ");
    fgets(password, sizeof(password), stdin);
    // Here you should send a registration request to the server and handle the response
    printf("Registration successful (demo).\n");
    // You can return to the main menu or continue to login automatically
}

void handle_login() {
    char username[64], password[64];
    printf("Enter username: ");
    fgets(username, sizeof(username), stdin);
    printf("Enter password: ");
    fgets(password, sizeof(password), stdin);
    // Here you should send a login request to the server and handle the response
    printf("Login successful (demo).\n");
    show_group_menu(username);
}

void handle_exit() {
    printf("Exiting...\n");
    // Here you should clean up resources if needed
    exit(0);
}

void show_group_menu(const char* username) {
    int choice;
    char group_name[64];
    while (1) {
        printf("\n=== Group Menu ===\n");
        printf("1. Create Group\n");
        printf("2. Join Group\n");
        printf("3. Leave Group\n");
        printf("4. Logout\n");
        printf("Choose option: ");
        scanf("%d", &choice);
        getchar(); // consume newline

        switch (choice) {
            case 1:
                handle_create_group(username);
                break;
            case 2:
                handle_join_group(username);
                break;
            case 3:
                handle_leave_group(username);
                break;
            case 4:
                handle_logout(username);
                return;
            default:
                printf("Invalid option. Try again.\n");
        }
    }
}

void handle_create_group(const char* username) {
    char group_name[64];
    printf("Enter new group name: ");
    fgets(group_name, sizeof(group_name), stdin);
    // Send a request to the server to create a group
    printf("Group created and joined (demo).\n");
    open_chat_windows(group_name, username);
}

void handle_join_group(const char* username) {
    char group_name[64];
    printf("Enter group name to join: ");
    fgets(group_name, sizeof(group_name), stdin);
    // Send a request to the server to join the group
    printf("Joined group (demo).\n");
    open_chat_windows(group_name, username);
}

void handle_leave_group(const char* username) {
    char group_name[64];
    printf("Enter group name to leave: ");
    fgets(group_name, sizeof(group_name), stdin);
    // Send a request to the server to leave the group
    printf("Left group (demo).\n");
}

void handle_logout(const char* username) {
    printf("Logging out...\n");
    // Send a request to the server to logout
}

void open_chat_windows(const char* group_name, const char* username) {
    printf("Opening chat windows for group: %s (demo)\n", group_name);
    // Here you should open processes/windows for sending and receiving messages
}

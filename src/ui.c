/*
 * ui.c - Terminal user interface implementation
 *
 * Simple text menus that read user input from stdin.
 */

#include "ui.h"
#include <stdio.h>
#include <string.h>

/*
 * read_line - Read one line from stdin into buf (up to max-1 chars).
 * Strips trailing newline. Returns 1 on success, 0 on EOF.
 */
static int read_line(const char *prompt, char *buf, size_t max)
{
    printf("%s", prompt);
    fflush(stdout);

    if (!fgets(buf, (int)max, stdin)) {
        return 0;
    }

    size_t len = strlen(buf);
    if (len > 0 && buf[len - 1] == '\n') {
        buf[len - 1] = '\0';
    }
    return 1;
}

/*
 * show_login_screen - Screen 1: register / login / exit
 */
int show_login_screen(char *out_username, char *out_password)
{
    char choice[8];

    while (1) {
        printf("\n========== Chat Client ==========\n");
        printf("  1. Register\n");
        printf("  2. Login\n");
        printf("  3. Exit\n");
        printf("=================================\n");

        if (!read_line("Choose: ", choice, sizeof(choice))) {
            return UI_EXIT;
        }

        int opt = choice[0] - '0';

        if (opt == 3) {
            return UI_EXIT;
        }

        if (opt == 1 || opt == 2) {
            if (!read_line("Username: ", out_username, MAX_USERNAME_LEN)) {
                return UI_EXIT;
            }
            if (!read_line("Password: ", out_password, MAX_PASSWORD_LEN)) {
                return UI_EXIT;
            }
            return (opt == 1) ? UI_REGISTER : UI_LOGIN;
        }

        printf("  Invalid option, try again.\n");
    }
}

/*
 * show_group_screen - Screen 2: create / join / leave / logout
 */
int show_group_screen(char *out_group_name)
{
    char choice[8];

    while (1) {
        printf("\n========== Group Menu ==========\n");
        printf("  1. Create group\n");
        printf("  2. Join group\n");
        printf("  3. Leave group\n");
        printf("  4. Logout\n");
        printf("================================\n");

        if (!read_line("Choose: ", choice, sizeof(choice))) {
            return UI_LOGOUT;
        }

        int opt = choice[0] - '0';

        switch (opt) {
            case 1:
                if (!read_line("Group name: ", out_group_name,
                               MAX_GROUP_NAME_LEN)) {
                    return UI_LOGOUT;
                }
                return UI_CREATE_GROUP;

            case 2:
                if (!read_line("Group name to join: ", out_group_name,
                               MAX_GROUP_NAME_LEN)) {
                    return UI_LOGOUT;
                }
                return UI_JOIN_GROUP;

            case 3:
                if (!read_line("Group name to leave: ", out_group_name,
                               MAX_GROUP_NAME_LEN)) {
                    return UI_LOGOUT;
                }
                return UI_LEAVE_GROUP;

            case 4:
                return UI_LOGOUT;

            default:
                printf("  Invalid option, try again.\n");
                break;
        }
    }
}

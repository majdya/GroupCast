#ifndef UI_H
#define UI_H

/*
 * ui.h - Terminal user interface
 *
 * Provides two menu screens:
 *   Screen 1: registration / login / exit
 *   Screen 2: create group / join group / leave group / logout
 */

#include "common.h"

/* Action codes returned by the UI functions */
typedef enum {
    UI_EXIT         = 0,
    UI_REGISTER     = 1,
    UI_LOGIN        = 2,
    UI_CREATE_GROUP = 3,
    UI_JOIN_GROUP   = 4,
    UI_LEAVE_GROUP  = 5,
    UI_LOGOUT       = 6
} UiAction;

/*
 * show_login_screen - Display Screen 1 (register / login / exit).
 * Fills out_username and out_password when user picks register or login.
 * Returns: UI_REGISTER, UI_LOGIN, or UI_EXIT.
 */
int show_login_screen(char *out_username, char *out_password);

/*
 * show_group_screen - Display Screen 2 (create / join / leave / logout).
 * Fills out_group_name when a group name is needed.
 * Returns: UI_CREATE_GROUP, UI_JOIN_GROUP, UI_LEAVE_GROUP, or UI_LOGOUT.
 */
int show_group_screen(char *out_group_name);

#endif /* UI_H */

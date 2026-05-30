#ifndef CLIENT_MNG_H
#define CLIENT_MNG_H

/*
 * client_mng.h - Server response handler
 *
 * Parses incoming TlvPackets from the server and prints results.
 * On JOIN/CREATE success, stores the multicast address + port
 * in a "pending" scratch pad for the main loop to pick up.
 */

#include "common.h"

/* Parse and handle a TlvPacket received from the server. */
void handle_server_response(const TlvPacket *pkt);

/* Clear the pending group info (call before each recv). */
void pending_group_clear(void);

/*
 * Check if the last response stored pending group info.
 * If yes, copies IP into ip_out and port into port_out, returns 1.
 * If no pending info, returns 0.
 */
int pending_group_get(char *ip_out, int *port_out);

#endif /* CLIENT_MNG_H */

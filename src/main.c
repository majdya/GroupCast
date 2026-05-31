#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define DEFAULT_PORT 8888

int main(int argc, char* argv[])
{
    uint16_t port = DEFAULT_PORT;
    if (argc > 1) {
        long p = atol(argv[1]);
        if (p > 0 && p <= 65535) port = (uint16_t)p;
    }

    Server* srv = Server_Create(port);
    if (!srv) {
        fprintf(stderr, "Failed to create server on port %d\n", port);
        return 1;
    }
    printf("Server listening on port %d...\n", port);
    Server_Run(srv);
    Server_Destroy(&srv);
    return 0;
}

CC      = gcc
CFLAGS  = -Wall -Werror -g -I ds -I src
LDLIBS  =

DS_SRC  = ds/gen_dlist.c ds/hash_map.c
SRC_SRC = src/protocol.c src/comm_link.c
SRV_SRC = src/server_mng.c src/group_mng.c src/server.c src/main.c

.PHONY: all test clean

all: server test

server: $(SRC_SRC) $(SRV_SRC) $(DS_SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

test: test_dlist test_hashmap test_comm test_e2e

test_dlist: tests/test_dlist.c ds/gen_dlist.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)
	./$@

test_hashmap: tests/test_hashmap.c $(DS_SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)
	./$@

test_comm: tests/test_comm.c $(SRC_SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)
	./$@

test_e2e: tests/test_e2e.c $(SRC_SRC) server
	$(CC) $(CFLAGS) $< $(SRC_SRC) -o $@ $(LDLIBS)
	./$@

clean:
	rm -f server test_dlist test_hashmap test_comm test_e2e

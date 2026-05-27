CC      = gcc
CFLAGS  = -Wall -g -I ds -I src -I include
LDLIBS  =

DS_SRC  = ds/gen_dlist.c ds/hash_map.c
SRC_SRC = src/protocol.c src/comm_link.c
SRV_SRC = src/server_mng.c src/group_mng.c src/server.c src/main.c
CLN_SRC = src/client_main.c src/client_net.c src/client_mng.c \
          src/client_groups_mng.c src/ui.c src/gen_dlist.c
RECV_SRC = src/chat_receiver.c
SEND_SRC = src/chat_sender.c

.PHONY: all test clean

all: server client chat_receiver chat_sender test

server: $(SRC_SRC) $(SRV_SRC) $(DS_SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

client: $(SRC_SRC) $(CLN_SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

chat_receiver: $(RECV_SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

chat_sender: $(SEND_SRC)
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
	rm -f server client chat_receiver chat_sender \
	      test_dlist test_hashmap test_comm test_e2e

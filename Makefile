CC      = gcc
CFLAGS  = -Wall -g -I ds -I src
LDLIBS  =

DS_SRC  = ds/gen_dlist.c ds/hash_map.c
SRC_SRC = src/protocol.c src/comm_link.c

.PHONY: all test clean

all: test

test: test_dlist test_hashmap test_comm

test_dlist: tests/test_dlist.c ds/gen_dlist.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)
	./$@

test_hashmap: tests/test_hashmap.c $(DS_SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)
	./$@

test_comm: tests/test_comm.c $(SRC_SRC)
	$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)
	./$@

clean:
	rm -f test_dlist test_hashmap test_comm

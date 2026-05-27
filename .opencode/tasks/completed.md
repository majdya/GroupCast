# Completed Tasks

## Phase 1 — Protocol & Infrastructure (2026-05-27)

- `src/types.h` — GroupInfo struct + max-size constants
- `src/protocol.h` — MessageType enum (12 codes), StatusCode enum (9 codes), TLV_MAX_PAYLOAD
- `src/protocol.c` — Pack/Unpack helpers: Str, UserPass, GroupResp
- `src/comm_link.h` — CommPeer opaque, CommResult enum
- `src/comm_link.c` — Comm_Listen/Accept/Connect/Close/Send/TryRecv/FD/FromFd, partial-read TCP reassembly
- `tests/test_comm.c` — 5 socketpair tests (send/recv, partial, multi, zero-payload, close)
- `Makefile` — test_dlist, test_hashmap, test_comm, clean

## Phase 0 — Data Structures Library (2026-05-27)

- `ds/gen_dlist.c` — sentinel-based doubly-linked list, all core ops + iterators
- `ds/hash_map.c` — separate chaining, lazy buckets, prime rounding, rehash, statistics
- `tests/test_dlist.c` — 34 tests (was 28, added edge cases: NULL begin, next/prev boundary)
- `tests/test_hashmap.c` — 32 tests (was 18, added NULL map, early ForEach stop, chain remove, edge cases)
- `ds/gen_dlist.c` fixes: ListItrBegin(NULL) returns NULL, ListItrNext(end)/ListItrPrev(begin) return self per contract
- `ds/hash_map.c` fixes: i <= n/i overflow guard, next_prime saturation guard

# Current Sprint — Phase 2: Server

**🔜 Next: 2.1 server_mng.c + 2.2 group_mng.c + 2.3 server.c**

## Phase 1 — Protocol & Infrastructure (done)

- [x] 1.1  `src/types.h` — GroupInfo struct
- [x] 1.2  `src/protocol.h / protocol.c` — MessageType/StatusCode enums, pack/unpack helpers
- [x] 1.3  `src/comm_link.h / comm_link.c` — CommPeer, Comm_Send, Comm_TryRecv, partial-read buffering
- [x] 1.4  `tests/test_comm.c` — 5 socketpair tests (send/recv, partial, multi, zero, close)
- [x] 1.5  `Makefile` — test_dlist, test_hashmap, test_comm, clean

## Phase 0 — ds/ implementation (done)

### gen_dlist.c — Core Operations
- [ ] 0.1  *header defined by user*
- [ ] 0.2  `ListCreate` — allocate sentinel list
- [ ] 0.3  `ListDestroy` — free all nodes + sentinels
- [ ] 0.4  `ListPushHead` — insert at front
- [ ] 0.5  `ListPopHead` — remove from front
- [ ] 0.6  `ListPopTail` — remove from back
- [ ] 0.7  `ListSize` — count elements
- [ ] 0.8  `ListIsEmpty` — check if empty

### gen_dlist.c — Iterators
- [ ] 0.9  `ListItrBegin` — first element
- [ ] 0.10 `ListItrEnd` — past-the-end sentinel
- [ ] 0.11 `ListItrNext` — advance
- [ ] 0.12 `ListItrPrev` — retreat
- [ ] 0.13 `ListItrGet` — read data
- [ ] 0.14 `ListItrSet` — write data
- [ ] 0.15 `ListItrInsertBefore` — insert before position
- [ ] 0.16 `ListItrRemove` — remove at position
- [ ] 0.17 `ListItrForEach` — apply action over range

### hash_map.c
- [ ] 0.18 `HashMap_Create` — allocate bucket array (prime rounding)
- [ ] 0.19 `HashMap_Destroy` — free all entries + buckets
- [ ] 0.20 `HashMap_Insert` — hash → find bucket → list push
- [ ] 0.21 `HashMap_Remove` — find key → list remove
- [ ] 0.22 `HashMap_Find` — hash → search bucket list
- [ ] 0.23 `HashMap_Size` — return item count
- [ ] 0.24 `HashMap_Rehash` — resize + redistribute
- [ ] 0.25 `HashMap_ForEach` — iterate all entries
- [ ] 0.26 `HashMap_GetStatistics` — debug stats

### tests/ — Pass 1 tests
- [ ] 0.27 `tests/test_dlist.c` — test all list ops + iterators
- [ ] 0.28 `tests/test_hashmap.c` — test all map ops

---

## Backlog

### Phase 1 — Protocol & Infrastructure
- [ ] 1.1  `src/types.h` — GroupInfo struct (name, mcast_addr, mcast_port)
- [ ] 1.2  `src/protocol.h` — MessageType enum (12 codes) + StatusCode enum (9 codes) + TLV_MAX_PAYLOAD
- [ ] 1.3  `src/comm_link.h / comm_link.c` — CommPeer opaque, Comm_Send, Comm_TryRecv (CommResult enum), Comm_Listen/Accept/Connect/Close/FD
- [ ] 1.4  `tests/test_comm.c` — socketpair-based TLV send/recv/partial/multi tests
- [ ] 1.5  `Makefile` — all targets + test targets, -Wall -g, -lrt

### Phase 2 — Server
- [ ] 2.1  `src/server_mng.c` — HandleRegister, HandleLogin, HandleLogout
- [ ] 2.2  `src/group_mng.c` — HandleCreate, HandleJoin, HandleLeave, MC pool
- [ ] 2.3  `src/server.c` — select() loop, accept, dispatch, disconnect

### Phase 3 — Client
- [ ] 3.1  `src/client_mng.c` — Register, Login, CreateGroup, JoinGroup, LeaveGroup, Logout wrappers
- [ ] 3.2  `src/ui.c` — Screen 1 + Screen 2 menus
- [ ] 3.3  `src/client.c` — TCP connect, UI loop, screen flow

### Phase 4 — Chat Windows & IPC
- [ ] 4.1  `src/chat_send.c` — UDP multicast sender
- [ ] 4.2  `src/chat_recv.c` — UDP multicast receiver
- [ ] 4.3  Client integration — spawn gnome-terminal, mqueue PID, kill on leave

---

## Pass 2 — Advanced (portfolio polish)

### Security
- [ ] P2.1  SHA256 password hashing via libcrypt
- [ ] P2.2  fork+execvp instead of system() for chat windows

### CLI & Config
- [ ] P2.3  getopt for server: `-p <port> -m <mc_range>`
- [ ] P2.4  getopt for client: `-a <addr> -p <port>`

### Robustness
- [ ] P2.5  Signal handling (SIGINT graceful shutdown)
- [ ] P2.6  Socket timeouts (SO_RCVTIMEO)

### Testing
- [ ] P2.7  tests/test_dlist.c
- [ ] P2.8  tests/test_hashmap.c
- [ ] P2.9  tests/test_protocol.c

### Polish
- [ ] P2.10 .clang-format
- [ ] P2.11 .github/workflows/ci.yml
- [ ] P2.12 README.md with architecture diagram + usage
- [ ] P2.13 LICENSE (MIT)

### Bonus
- [ ] P2.14 ncurses TUI
- [ ] P2.15 Multicast message encryption

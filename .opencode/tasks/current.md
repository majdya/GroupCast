# Current Sprint — Phase 0: Data Structures Library

## Phase 0 — ds/ implementation

### gen_dlist.c — Core Operations
- [ ] 0.1  `ListCreate` — allocate sentinel list
- [ ] 0.2  `ListDestroy` — free all nodes + sentinels
- [ ] 0.3  `ListPushHead` — insert at front
- [ ] 0.4  `ListPushTail` — insert at back
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

---

## Backlog

### Phase 1 — Protocol & Infrastructure
- [ ] 1.1  `src/types.h` — ClientState, Group structs
- [ ] 1.2  `src/protocol.h` — TLV type enums + status enums
- [ ] 1.3  `src/comm_link.h / comm_link.c` — Comm_Listen, Accept, Connect, Send, Recv, Close, FD
- [ ] 1.4  `Makefile` — all targets, -Wall -g, -lrt

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

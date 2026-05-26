# GroupCast — Project Context

## Overview
Real-time group chat on LAN using **TCP** (management) + **UDP multicast** (messaging).
University Data Communication course project. C language.

## Current Focus
- **Phase**: 0 (Data Structures Library)
- **Pass**: 1 (Base — working MVP)
- **Active files**: `ds/gen_dlist.c`, `ds/hash_map.c`
- **Next action**: Implement `ListCreate` + `ListDestroy`
- **Blockers**: None
- **Known issues**: None

## Architecture

```
CLIENT                                SERVER
──────                                ──────
Screen 1 (Register/Login/Exit)        select() loop
Screen 2 (Create/Join/Leave/Logout)    ├── Comm_Accept()
       │                                ├── Comm_Recv() → dispatch:
       ▼                                    ├── ServerMng (User Hash)
  ClientMng                                   │   register / login / logout
       │                                      └── GroupMng (Group Hash)
       ▼                                          create / join / leave / free MC IP
  Comm-Link (TLV framing)
       │
       ▼
  TCP socket
```

## Team
- **Dev A**: Server (Phase 2) + shared infrastructure
- **Dev B**: Client + UI (Phase 3) + shared infrastructure
- Both: Chat windows (Phase 4)

## Development Strategy

### Pass 1 — Base (working MVP)
Make it work end-to-end. Full demo flow:
`register → login → create group → join → chat → leave → logout`

### Pass 2 — Advanced (portfolio polish)
Security, robustness, testing, CI, documentation.

## File Map

```
GroupCast/
├── AGENTS.md
├── .opencode/tasks/current.md
├── .opencode/tasks/completed.md
├── Makefile
├── README.md                          (Pass 2)
├── LICENSE                            (Pass 2)
├── .clang-format                      (Pass 2)
├── .github/workflows/ci.yml           (Pass 2)
├── ds/
│   ├── gen_dlist.h                    (done)
│   ├── gen_dlist.c                    (Phase 0)
│   ├── hash_map.h                     (exists)
│   └── hash_map.c                     (Phase 0)
├── src/
│   ├── types.h                        (Phase 1)
│   ├── protocol.h                     (Phase 1)
│   ├── comm_link.h / comm_link.c      (Phase 1)
│   ├── server.h / server.c            (Phase 2)
│   ├── server_mng.h / server_mng.c    (Phase 2)
│   ├── group_mng.h / group_mng.c      (Phase 2)
│   ├── client.h / client.c            (Phase 3)
│   ├── client_mng.h / client_mng.c    (Phase 3)
│   ├── ui.h / ui.c                    (Phase 3)
│   ├── chat_send.c                    (Phase 4)
│   └── chat_recv.c                    (Phase 4)
└── tests/                             (Pass 2)
```

## Pass 2 Upgrades (not started)
- SHA256 password hashing
- fork+execvp instead of system()
- CLI args via getopt
- Signal handling (SIGINT graceful shutdown)
- Socket timeouts
- Unit tests (dlist, hashmap, protocol)
- .clang-format
- GitHub CI
- README with architecture diagram
- MIT License
- ncurses TUI (bonus)
- Multicast encryption (bonus)

## Protocol — TLV Format
- **Type**: 1 byte
- **Length**: 2 bytes (network byte order)
- **Value**: N bytes

### Message Types
| Code | Name | Description |
|------|------|-------------|
| 0x01 | REGISTER_REQ | Register new user |
| 0x02 | REGISTER_RESP | Registration result |
| 0x03 | LOGIN_REQ | User login |
| 0x04 | LOGIN_RESP | Login result |
| 0x05 | CREATE_GROUP_REQ | Create new group |
| 0x06 | CREATE_GROUP_RESP | Group creation result |
| 0x07 | JOIN_GROUP_REQ | Join existing group |
| 0x08 | JOIN_GROUP_RESP | Join result (includes MC addr+port) |
| 0x09 | LEAVE_GROUP_REQ | Leave group |
| 0x0A | LEAVE_GROUP_RESP | Leave result |
| 0x0B | LOGOUT_REQ | Logout |
| 0x0C | LOGOUT_RESP | Logout result |

### Status Codes
| Code | Name |
|------|------|
| 0 | SUCCESS |
| 1 | ERR_USER_EXISTS |
| 2 | ERR_USER_NOT_FOUND |
| 3 | ERR_WRONG_PASSWORD |
| 4 | ERR_GROUP_EXISTS |
| 5 | ERR_GROUP_NOT_FOUND |
| 6 | ERR_ALREADY_IN_GROUP |
| 7 | ERR_NOT_IN_GROUP |
| 8 | ERR_GENERAL |

## Data Structures

### ds/gen_dlist.h — Generic Doubly-Linked List
Sentinel-based design. Internal:
```c
struct Node { void* m_data; Node* m_next; Node* m_prev; };
struct List { Node m_head; Node m_tail; };  // sentinel nodes
```

### ds/hash_map.h — HashMap
Separate chaining using linked lists. Bucket count rounded to nearest prime.

## Decision Log

| Date | Decision | Rationale |
|------|----------|-----------|
| 2026-05-26 | `select()` event loop over threads | Single-threaded, no locks, sufficient for LAN scale |
| 2026-05-26 | Comm-Link abstraction layer | Isolates networking from business logic; matches block diagram |
| 2026-05-26 | TLV framing on TCP | Length prefix solves TCP stream boundary problem |
| 2026-05-26 | HashMap + List from project spec | Mandated; generic C containers show data structure competency |
| 2026-05-26 | Two-pass strategy (Base → Advanced) | Working demo first, then polish for portfolio |
| 2026-05-26 | `system()` gnome-terminal in Base | Per spec; upgrade to `fork+execvp` in Advanced |
| 2026-05-26 | POSIX message queues for IPC | Standard Linux IPC; PID passing between processes |
| 2026-05-26 | Multicast range 239.255.0.0/24 | Administratively scoped, LAN-only, no upstream leakage |
| 2026-05-26 | AGENTS.md + .opencode/tasks/ for context | Persistent project memory across sessions; auto-loaded on opencode start |

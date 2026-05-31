# GroupCast — Testing Guide

## Quick Start

```bash
make clean && make test
```

This builds and runs every test. Exit code 0 = all pass.

## Test Suites

| Test file | Layer | Type | Tests | What it covers |
|-----------|-------|------|-------|----------------|
| `tests/test_dlist.c` | ds/ | Unit | 34 | Doubly-linked list: create/destroy, push/pop head/tail, size, empty, FIFO/LIFO order, iterators (forward/backward/begin/end/next/prev/get/set), insert-before, remove, ForEach (full + early-stop), NULL-safety for iterator boundary conditions |
| `tests/test_hashmap.c` | ds/ | Unit | 32 | Hash map: create/destroy, insert/find, duplicate key rejection, missing key handling, remove (head/middle/tail of chain), rehash preserves entries, NULL key rejection, NULL-map safety, ForEach (visit all + early stop + empty), statistics |
| `tests/test_comm.c` | comm_link | Integration | 5 | TLV framing over UNIX socketpair: roundtrip send/recv, partial-read reassembly (byte-at-a-time), multi-message sequencing, zero-length payload, peer-disconnect detection |
| `tests/test_e2e.c` | server | E2E | 6 scenario groups | Full protocol over TCP against a live server process: register, login, create/join/leave groups, error paths, logout, multi-session interop |

### Total: **77+ tests** across 4 suites.

## e2e Scenarios in Detail

`test_e2e.c` spawns a real server process (fork+exec) and talks to it over TCP on port 18989. Each scenario exercises a specific protocol flow:

| Scenario | Steps verified |
|----------|----------------|
| **Register + Login** | Register alice → SUCCESS; duplicate register → ERR_USER_EXISTS; login → SUCCESS |
| **Login errors** | Wrong password → ERR_WRONG_PASSWORD; non-existent user → ERR_USER_NOT_FOUND |
| **Group CRUD** | Create "devs" → SUCCESS with MC addr/port; duplicate → ERR_GROUP_EXISTS; join → MC addr/port returned; already joined → ERR_ALREADY_IN_GROUP; leave → SUCCESS; leave again → ERR_NOT_IN_GROUP; join non-existent → ERR_GROUP_NOT_FOUND |
| **Logout** | Logout → SUCCESS |
| **Requires login** | Create/Join/Leave without login → ERR_GENERAL |
| **Multi-session** | Bob connects separately, registers, logs in, joins "devs", leaves (same group as Alice) |

## Testing Strategy

```
┌─────────────────────────────────────────────────────┐
│                    E2E (test_e2e)                    │
│           Full server process over TCP               │
├─────────────────────────────────────────────────────┤
│           Integration (test_comm)                    │
│         Comm-Link layer via UNIX socketpair          │
├─────────────────────────────────────────────────────┤
│              Unit (test_dlist, test_hashmap)         │
│          Data structures in isolation                │
└─────────────────────────────────────────────────────┘
```

- **Unit tests** — Validate data structure contracts (gen_dlist, hash_map) in complete isolation. No I/O.
- **Integration tests** — Validate the comm_link framing layer with socketpair. No server logic involved.
- **E2E tests** — Validate the full server stack through the actual TCP protocol. Exercises request dispatch, session management, user/group state machines, and multi-client interop.

## Adding New Tests

1. Pick the right file for the layer you're testing.
2. Follow the existing pattern:
   - Use `TEST("description")`, `PASS()`, `FAIL(msg)`, `ASSERT(cond, msg)` macros.
   - Keep scenarios self-contained — no shared mutable state between functions (each starts fresh or builds from a known state).
3. Add the test target to the Makefile (follow existing `test_xxx` pattern).
4. Add the target to the `test:` phony dependency list.
5. Run `make test` — all suites must pass.

### Adding a new e2e scenario

- Open `tests/test_e2e.c`, write a new `static void test_xxx(CommPeer* peer)` function.
- Call it from `main()` between setup and teardown.
- The test has a connected, logged-in (or not) peer — choose the right starting state.
- For multi-peer scenarios, call `Comm_Connect` directly inside the test function.

# Makefile - Chat Client Project
#
# Builds three binaries:
#   client        - the main chat client
#   chat_receiver - standalone multicast receiver (opened in gnome-terminal)
#   chat_sender   - standalone multicast sender (opened in gnome-terminal)

CC      = gcc
CFLAGS  = -Wall -Wextra -pedantic -g
LDFLAGS =

# ---- Main client ----
CLIENT_SRC = client_main.c client_net.c client_mng.c \
             client_groups_mng.c ui.c protocol.c gen_dlist.c
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)
CLIENT_BIN = client

# ---- Chat receiver (standalone) ----
RECV_SRC = chat_receiver.c
RECV_OBJ = $(RECV_SRC:.c=.o)
RECV_BIN = chat_receiver

# ---- Chat sender (standalone) ----
SEND_SRC = chat_sender.c
SEND_OBJ = $(SEND_SRC:.c=.o)
SEND_BIN = chat_sender

# ---- Targets ----
all: $(CLIENT_BIN) $(RECV_BIN) $(SEND_BIN)

$(CLIENT_BIN): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(RECV_BIN): $(RECV_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(SEND_BIN): $(SEND_OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# ---- Compile rules ----
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

# ---- Clean ----
clean:
	rm -f *.o $(CLIENT_BIN) $(RECV_BIN) $(SEND_BIN)

.PHONY: all clean

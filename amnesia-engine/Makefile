CC = gcc
CFLAGS = -Wall -Wextra -O2
LIBS = -lsodium

TARGET = amnesia_engine

SRCS = src/main.c src/vault.c src/ipc.c src/netty.c src/engine.c

all:
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LIBS)

clean:
	rm -f $(TARGET)
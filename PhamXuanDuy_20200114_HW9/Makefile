CC = gcc
CFLAGS = -Wall
LDFLAGS_SERVER = -pthread -lssl -lcrypto
LDFLAGS_CLIENT = -pthread -lssl -lcrypto

all: server client

server: server.c
	$(CC) $(CFLAGS) -o server server.c $(LDFLAGS_SERVER)

client: client.c
	$(CC) $(CFLAGS) -o client client.c $(LDFLAGS_CLIENT)

clean:
	rm -f server client

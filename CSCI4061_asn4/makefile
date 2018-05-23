CC=/usr/bin/gcc-5
CFLAGS=-std=c99 -pthread -w
DBFLAGS=-g -pthread

make: server.c client.c
	$(CC) $(CFLAGS) -o server server.c
	$(CC) $(CFLAGS) -o client client.c

server: server.c
	$(CC) $(CFLAGS) -o server server.c

client: client.c
	$(CC) $(CFLAGS) -o client client.c

clean: 
	rm server client

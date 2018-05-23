CC=gcc
CFLAGS=-std=c99 -w -pthread
DBFLAGS=-std=c99 -pthread

make: votecounter.c 
	$(CC) $(CFLAGS) -o votecounter votecounter.c

debug: votecounter.c
	$(CC) $(DBFLAGS) -o votecounter votecounter.c

clean: 
	rm votecounter

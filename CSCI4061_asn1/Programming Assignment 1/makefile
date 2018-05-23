CC=/usr/bin/gcc
CFLAGS=-std=c99 -w
DBFLAGS=-g

make: votecounter.c 
	$(CC) $(CFLAGS) -o votecounter votecounter.c

debug: votecounter.c
	$(CC) $(CFLAGS) $(DBFLAGS) -o votecounter votecounter.c

clean: 
	rm votecounter

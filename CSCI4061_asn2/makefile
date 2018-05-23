CC=/usr/bin/gcc
CFLAGS=-std=c99 -w
DBFLAGS=-g

make: Vote_Counter.c Leaf_Counter.c Aggregate_Votes.c
	$(CC) $(CFLAGS) -o Vote_Counter Vote_Counter.c
	$(CC) $(CFLAGS) -o Leaf_Counter Leaf_Counter.c
	$(CC) $(CFLAGS) -o Aggregate_Votes Aggregate_Votes.c

Vote_Counter: Vote_Counter.c 
	$(CC) $(CFLAGS) -o Vote_Counter Vote_Counter.c

Leaf_Counter: Leaf_Counter.c
	$(CC) $(CFLAGS) -o Leaf_Counter Leaf_Counter.c

Aggregate_Votes: Aggregate_Votes.c
	$(CC) $(CFLAGS) -o Aggregate_Votes Aggregate_Votes.c

clean: 
	rm Aggregate_Votes Leaf_Counter Vote_Counter

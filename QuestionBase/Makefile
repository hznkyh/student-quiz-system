CC=gcc
CFLAGS=-Wall -std=c11

.PHONY: all clean

all: runQB

QB.o: QB.c QB.h
	$(CC) $(CFLAGS) -c QB.c

runQB.o: runQB.c QB.h
	$(CC) $(CFLAGS) -c runQB.c

runQB: QB.o runQB.o
	$(CC) $(CFLAGS) QB.o runQB.o -o runQB

clean:
	rm -f *.o runQB
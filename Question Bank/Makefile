CC = gcc
CFLAGS = -Wall -Wextra

QB: QB.o
	$(CC) $(CFLAGS) -o QB QB.o

QB.o: QB.c QB.h
	$(CC) $(CFLAGS) -c QB.c

.PHONY: clean
clean:
	rm -f QB.o QB
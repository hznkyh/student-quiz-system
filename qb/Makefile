CC = gcc
CFLAGS = -Wall

QBP: python_qb/QBP.c python_qb/QB.h
	$(CC) $(CFLAGS) -o python_qb/QBP python_qb/QBP.c
QBC: c_qb/QBC.c c_qb/QB.h
	$(CC) $(CFLAGS) -o c_qb/QBC c_qb/QBC.c

.PHONY: clean
clean:
	rm -f QBP 
	rm -f QBC 
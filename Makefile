CC=gcc

CFLAGS=-Wall -Wextra -O0 -g -std=c11 -pthread #-Werror para versao final
LDFLAGS=-lm

.PHONY: all

all: grade

priothreads.o: priothreads.h priothreads.c
	$(CC) $(CFLAGS) -c priothreads.c

test: priothreads.o test.c 
	$(CC) $(CFLAGS) -o test priothreads.o test.c $(LDFLAGS) 

grade: test
	./test

clean:
	rm -rf *.o test

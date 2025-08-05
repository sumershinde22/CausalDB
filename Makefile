CC=gcc
CFLAGS=-Wall -Wextra -std=c99
LIBS=-lsqlite3

all: causaldb

causaldb: main.c db.c event.c statement.c repl.c
	$(CC) $(CFLAGS) -o causaldb main.c db.c event.c statement.c repl.c

benchmark: benchmark.c db.c event.c
	$(CC) $(CFLAGS) -o benchmark benchmark.c db.c event.c $(LIBS)

clean:
	rm -f causaldb benchmark *.o *.cdb *.db
	cd server && make clean

.PHONY: all clean benchmark
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -I../include
SERVER_OBJS = server.o ../build/db.o ../build/event.o ../build/statement.o

server: $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o server $(SERVER_OBJS)

server.o: server.c
	$(CC) $(CFLAGS) -c server.c

../build/db.o: ../src/db.c ../include/db.h
	@mkdir -p ../build
	$(CC) $(CFLAGS) -c ../src/db.c -o ../build/db.o

../build/event.o: ../src/event.c ../include/event.h
	@mkdir -p ../build
	$(CC) $(CFLAGS) -c ../src/event.c -o ../build/event.o

../build/statement.o: ../src/statement.c ../include/statement.h
	@mkdir -p ../build
	$(CC) $(CFLAGS) -c ../src/statement.c -o ../build/statement.o

clean:
	rm -f *.o server

.PHONY: clean 
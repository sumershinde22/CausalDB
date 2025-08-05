CC = gcc
CFLAGS = -Wall -Wextra -std=c99
SERVER_OBJS = server.o ../db.o ../event.o ../statement.o

server: $(SERVER_OBJS)
	$(CC) $(CFLAGS) -o server $(SERVER_OBJS)

server.o: server.c
	$(CC) $(CFLAGS) -c server.c

../db.o: ../db.c ../db.h
	$(CC) $(CFLAGS) -c ../db.c -o ../db.o

../event.o: ../event.c ../event.h
	$(CC) $(CFLAGS) -c ../event.c -o ../event.o

../statement.o: ../statement.c ../statement.h
	$(CC) $(CFLAGS) -c ../statement.c -o ../statement.o

clean:
	rm -f *.o server ../*.o

.PHONY: clean 
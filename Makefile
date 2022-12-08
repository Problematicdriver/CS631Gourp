CC=cc
CFLAGS=-Wall -g -Werror -Wextra

default: sws
all: default

sws: socket.o sws.o reader.o writer.o
	$(CC) $(CFLAGS) -lmagic -o sws sws.o socket.o reader.o writer.o -lm

*.o: *.c *.h
	$(CC) $(CFLAGS) -c *.c

clean:
	rm -rf $(TARGET) *.o
	rm -rf sws

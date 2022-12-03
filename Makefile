CC=cc
CFLAGS=-Wall -Werror -Wextra

default: sws
all: default

sws: socket.o sws.o reader.o writer.o
	$(CC) $(CFLAGS) -o sws sws.o socket.o reader.o writer.o -lm

*.o: *.c *.h
	$(CC) $(CFLAGS) -c *.c

clean:
	rm -rf $(TARGET) *.o
	rm -rf sws

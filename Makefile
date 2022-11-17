CC=cc
CFLAGS=-Wall -Werror -Wextra

default: sws
all: default

sws: socket.o sws.o reader.o
	$(CC) $(CFLAGS) -o sws sws.o socket.o reader.o -lm

sws.o: sws.c sws.h
	$(CC) $(CFLAGS) -c sws.c

socket.o: socket.c socket.h
	$(CC) $(CFLAGS) -c socket.c

reader.o: reader.c reader.h
	$(CC) $(CFLAGS) -c reader.c

clean:
	rm -rf $(TARGET) *.o
	rm -rf sws

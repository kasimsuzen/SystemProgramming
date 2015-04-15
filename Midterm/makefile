CC=gcc
CFLAGS=-c -Wall
LDFLAGS= -lm

all: server client

server: syncrohyzationServer.c
	$(CC) -o $@ $^ $(LDFLAGS)

client: client.c
	$(CC) -o $@ $^

clean:
	-rm -f *.o core *.core $(EXECUTABLE)
CC=gcc
CFLAGS=-c -Wall
LDFLAGS= -pthread
SOURCES=uniqueWordCountThreadAndFork.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=uniqueWordCountThreadAndFork.e

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm -f *.o core *.core $(EXECUTABLE)
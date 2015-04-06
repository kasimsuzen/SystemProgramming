CC=gcc
CFLAGS=-c -Wall
LDFLAGS=
SOURCES=uniqueWordCountFork.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=uniqueWordCountFork.e

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm -f *.o core *.core $(EXECUTABLE)
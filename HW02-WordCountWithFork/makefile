CC=gcc
CFLAGS=-c -Wall
LDFLAGS=
SOURCES=wordCountFork.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=wordCountFork.e

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm -f *.o core *.core $(EXECUTABLE)
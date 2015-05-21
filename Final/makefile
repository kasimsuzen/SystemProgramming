CC=gcc
CFLAGS=-c -Wall
LDFLAGS= -pthread
SOURCES=mine.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=mine.e

all: $(SOURCES) $(EXECUTABLE)
    
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	-rm -f *.o core *.core $(EXECUTABLE)
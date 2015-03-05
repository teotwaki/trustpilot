TARGET = solver
LIBS = -lzmq -ljson-c
CC = gcc
CFLAGS = -m64 -std=c99 -pedantic -Wall -Wshadow -Wpointer-arith -Wcast-qual \
         -Wstrict-prototypes -Wmissing-prototypes -Werror -Wextra -O3 -g
GRIND = valgrind
GRIND_OPTS = --leak-check=full

.PHONY: default all clean grind

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.c, %.o, $(wildcard *.c))
HEADERS = $(wildcard *.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f *.o
	-rm -f $(TARGET)

grind: default
	$(GRIND) $(GRIND_OPTS) ./$(TARGET)

TARGET = solver
LIBS = -lzmq -ljson-c
CC = gcc
CFLAGS = -m64 -std=gnu11 -pedantic -Wall -Wshadow -Wpointer-arith -Wcast-qual \
         -Wstrict-prototypes -Wmissing-prototypes -Werror -Wextra -O3 -g
GRIND = valgrind
GRIND_OPTS = --leak-check=full
SRC_DIR = src

.PHONY: default all clean grind run

default: $(TARGET)
all: default

OBJECTS = $(patsubst %.c, %.o, $(wildcard $(SRC_DIR)/*.c))
HEADERS = $(wildcard $(SRC_DIR)/*.h)

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

.PRECIOUS: $(TARGET) $(OBJECTS)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -Wall $(LIBS) -o $@

clean:
	-rm -f $(SRC_DIR)/*.o
	-rm -f $(TARGET)

grind: default
	$(GRIND) $(GRIND_OPTS) ./$(TARGET)

run: default
	./$(TARGET)

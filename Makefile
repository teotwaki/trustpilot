TARGET     := solver
LIBS       := -lzmq -ljson-c -lpthread -lcrypto
CC         := gcc
CFLAGS     := -m64 -std=gnu11 -pedantic -Wall -Wshadow -Wpointer-arith \
              -Wcast-qual -Wstrict-prototypes -Wmissing-prototypes -Werror \
              -Wextra -O3 -g -Isrc
LFLAGS     := -Wall
GRIND      := valgrind
GRIND_OPTS := --leak-check=full
SRC_DIR    := src
TEST_DIR   := tests
TESTS      := test_is_anagram

.PHONY: default all clean grind run tests

all: $(TARGET)

OBJECTS      := $(SRC_DIR)/client.o $(SRC_DIR)/list.o $(SRC_DIR)/log.o \
                $(SRC_DIR)/solver.o
MAIN_OBJECTS := $(SRC_DIR)/main.o
TEST_OBJECTS := $(patsubst %,$(TEST_DIR)/%.o,$(TESTS))

.PRECIOUS: $(TARGET) $(TESTS) $(OBJECTS) $(MAIN_OBJECTS) $(TEST_OBJECTS)

$(TARGET): $(OBJECTS) $(MAIN_OBJECTS)
	$(CC) $^ $(LFLAGS) $(LIBS) -o $@

clean:
	-rm -f $(SRC_DIR)/*.o $(TEST_DIR)/*.o
	-rm -f $(TARGET) $(TESTS)

grind: all
	$(GRIND) $(GRIND_OPTS) ./$(TARGET)

run: all
	./$(TARGET)

tests: $(TESTS)

test_%: $(OBJECTS) $(TEST_DIR)/test_%.o
	$(CC) $^ $(CFLAGS) -Wall $(LIBS) -o $@

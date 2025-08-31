INCLUDE_DIR = include
LIB_DIR = lib

TEST_PATH = examples/test/server
TEST_PORT = 1337
ECHO_PATH = examples/echo/echo

INCLUDE_FILES = \
	$(shell find -type f -name '*.inc')

INCLUDE_DIRS = \
	$(shell find $(INCLUDE_DIR) -type d -printf '-Iinclude/%P ')

LIB_DIRS = $(shell find $(LIB_DIR) -type d -printf '-Ilib/%P ')

INCLUDE_FLAGS = $(INCLUDE_DIRS) $(LIB_DIRS)

DEBUG_FLAGS = -g
BASE_FLAGS = -felf64 -w+all

test:
	nasm -o $(TEST_PATH).o $(TEST_PATH).s \
		$(BASE_FLAGS) $(INCLUDE_FLAGS) $(DEBUG_FLAGS)
	ld -o $(TEST_PATH) $(TEST_PATH).o

echo:
	nasm -o $(ECHO_PATH).o $(ECHO_PATH).s \
		$(BASE_FLAGS) $(INCLUDE_FLAGS) $(DEBUG_FLAGS)
	ld -o $(ECHO_PATH) $(ECHO_PATH).o

.PHONY: clean
clean:
	rm -f $(ECHO_PATH) $(ECHO_PATH).o
	rm -f $(TEST_PATH) $(TEST_PATH).o


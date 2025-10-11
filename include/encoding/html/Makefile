INCLUDE_DIR = ../..
LIB_DIR = ../../../lib

TEST_PATH = tests/html_test

INCLUDE_DIRS = \
	$(shell find $(INCLUDE_DIR) -type d -printf '-I$(INCLUDE_DIR)/%P ')

LIB_DIRS = $(shell find $(LIB_DIR) -type d -printf '-I$(LIB_DIR)/%P ')

INCLUDE_FLAGS = $(INCLUDE_DIRS) $(LIB_DIRS)

DEBUG_FLAGS = -g
BASE_FLAGS = -felf64 -w+all

all:
	nasm -o $(TEST_PATH).o $(TEST_PATH).s \
		$(BASE_FLAGS) $(INCLUDE_FLAGS) $(DEBUG_FLAGS)
	ld -o $(TEST_PATH) $(TEST_PATH).o
	./$(TEST_PATH)

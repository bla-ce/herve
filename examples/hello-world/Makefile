ROOT_DIR = ../..
INCLUDE_DIR = $(ROOT_DIR)/include
LIB_DIR = $(ROOT_DIR)/lib
BUILD_DIR = build
BIN_DIR = bin
SRC_DIR = src

HELLO_WORLD = main

INCLUDE_DIRS = \
	$(shell find $(INCLUDE_DIR) -type d -printf '-I$(INCLUDE_DIR)/%P ')

LIB_DIRS = $(shell find $(LIB_DIR) -type d -printf '-I$(LIB_DIR)/%P ')

INCLUDE_FLAGS = $(INCLUDE_DIRS) $(LIB_DIRS)

DEBUG_FLAGS = -g
BASE_FLAGS = -felf64 -w+all

echo:
	mkdir -p $(BUILD_DIR) $(BIN_DIR)
	nasm -o $(BUILD_DIR)/$(HELLO_WORLD).o $(SRC_DIR)/$(HELLO_WORLD).s \
		$(INCLUDE_FLAGS) $(DEBUG_FLAGS) $(BASE_FLAGS)
	ld -o $(BIN_DIR)/$(HELLO_WORLD) $(BUILD_DIR)/$(HELLO_WORLD).o

run:
	./$(BIN_DIR)/$(HELLO_WORLD)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

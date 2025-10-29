ROOT_DIR = ../..
INCLUDE_DIR = $(ROOT_DIR)/include
LIB_DIR = $(ROOT_DIR)/lib
BUILD_DIR = build
BIN_DIR = bin
SRC_DIR = src

EXEC = main

INCLUDE_DIRS = \
	$(shell find $(INCLUDE_DIR) -type d -printf '-I$(INCLUDE_DIR)/%P ')

LIB_DIRS = $(shell find $(LIB_DIR) -type d -printf '-I$(LIB_DIR)/%P ')

SRC_DIRS = $(shell find $(SRC_DIR) -type d -printf '-I$(SRC_DIR)/%P ')

INCLUDE_FLAGS = $(INCLUDE_DIRS) $(LIB_DIRS) $(SRC_DIRS)

DEBUG_FLAGS = -g
BASE_FLAGS = -felf64 -w+all

echo:
	mkdir -p $(BUILD_DIR) $(BIN_DIR)
	nasm -o $(BUILD_DIR)/$(EXEC).o $(SRC_DIR)/$(EXEC).s \
		$(INCLUDE_FLAGS) $(DEBUG_FLAGS) $(BASE_FLAGS)
	ld -o $(BIN_DIR)/$(EXEC) $(BUILD_DIR)/$(EXEC).o

run:
	./$(BIN_DIR)/$(EXEC)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

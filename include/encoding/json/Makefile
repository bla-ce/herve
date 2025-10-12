INCLUDE_DIR = ../..
LIB_DIR = ../../../lib

BIN_DIR = bin
BUILD_DIR = build

JSON_PATH = tests/json_test

INCLUDE_DIRS = \
	$(shell find $(INCLUDE_DIR) -type d -printf '-I$(INCLUDE_DIR)/%P ')

LIB_DIRS = $(shell find $(LIB_DIR) -type d -printf '-I$(LIB_DIR)/%P ')

INCLUDE_FLAGS = $(INCLUDE_DIRS) $(LIB_DIRS)

DEBUG_FLAGS = -g
BASE_FLAGS = -felf64 -w+all

all:
	mkdir -p $(BIN_DIR) $(BUILD_DIR)
	nasm -o $(BUILD_DIR)/$(JSON_PATH).o $(JSON_PATH).s \
		$(BASE_FLAGS) $(INCLUDE_FLAGS) $(DEBUG_FLAGS)
	ld -o $(BIN_DIR)/$(JSON_PATH) $(BUILD_DIR)/$(JSON_PATH).o
	./$(JSON_PATH)

clean:
	rm -rf $(BIN_DIR) $(BUILD_DIR)

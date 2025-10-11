INCLUDE_DIR = include
LIB_DIR = lib
SRC_DIR = src
BIN_DIR = bin
BUILD_DIR = build

HERVE_PATH = herve

INCLUDE_FILES = \
	$(shell find -type f -name '*.inc')

INCLUDE_DIRS = \
	$(shell find $(INCLUDE_DIR) -type d -printf '-I$(INCLUDE_DIR)/%P ')

LIB_DIRS = $(shell find $(LIB_DIR) -type d -printf '-I$(LIB_DIR)/%P ')

INCLUDE_FLAGS = $(INCLUDE_DIRS) $(LIB_DIRS)

DEBUG_FLAGS = -g
BASE_FLAGS = -felf64 -w+all

herve:
	mkdir -p $(BIN_DIR) $(BUILD_DIR)
	nasm -o $(BUILD_DIR)/$(HERVE_PATH).o $(SRC_DIR)/$(HERVE_PATH).s \
		$(BASE_FLAGS) $(INCLUDE_FLAGS) $(DEBUG_FLAGS) 
	ld -o $(BIN_DIR)/$(HERVE_PATH) $(BUILD_DIR)/$(HERVE_PATH).o

run:
	./$(BIN_DIR)/$(HERVE_PATH)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR).o


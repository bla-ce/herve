ROOT_DIR = ../../..
INCLUDE_DIR = $(ROOT_DIR)/include
LIB_DIR = $(ROOT_DIR)/lib

BIN_DIR = bin
BUILD_DIR = build

TEST_FILES = $(wildcard *.s)

TARGETS := $(patsubst %.s,%,$(TEST_FILES))

INCLUDE_DIRS = \
	$(shell find $(INCLUDE_DIR) -type d -printf '-I$(INCLUDE_DIR)/%P ')

LIB_DIRS = $(shell find $(LIB_DIR) -type d -printf '-I$(LIB_DIR)/%P ')

INCLUDE_FLAGS = $(INCLUDE_DIRS) $(LIB_DIRS)

DEBUG_FLAGS = -g
BASE_FLAGS = -felf64 -w+all

all: $(TARGETS)

$(TARGETS): %: $(BUILD_DIR)/%.o
	mkdir -p $(BIN_DIR)
	ld -o $(BIN_DIR)/$@ $<

$(BUILD_DIR)/%.o: %.s
	mkdir -p $(BUILD_DIR)
	nasm -o $@ $< $(BASE_FLAGS) $(INCLUDE_FLAGS) $(DEBUG_FLAGS)

clean:
	rm -rf $(BIN_DIR) $(BUILD_DIR)

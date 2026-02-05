INCLUDE_DIR = include
LIB_DIR = lib
BUILD_DIR = build
BIN_DIR = bin
SRC_DIR = src
SVC_IMPL_DIR = svc_impl

MAIN_PATH = herve

INCLUDE_DIRS = \
	$(shell find $(INCLUDE_DIR) -type d -printf '-I$(INCLUDE_DIR)/%P ')

LIB_DIRS = $(shell find $(LIB_DIR) -type d -printf '-I$(LIB_DIR)/%P ')
SRC_DIRS = $(shell find $(SRC_DIR) -type d -printf '-I$(SRC_DIR)/%P ')
SVC_IMPL_DIRS = $(shell find $(SVC_IMPL_DIR) -type d -printf '-I$(SVC_IMPL_DIR)/%P ')

INCLUDE_FLAGS = $(INCLUDE_DIRS) $(LIB_DIRS) $(SRC_DIRS) $(SVC_IMPL_DIRS)

DEBUG_FLAGS = -g
BASE_FLAGS = -felf64 -w+all

herve:
	mkdir -p $(BUILD_DIR) $(BIN_DIR)
	nasm -o $(BUILD_DIR)/$(MAIN_PATH).o $(SRC_DIR)/$(MAIN_PATH).s \
		$(INCLUDE_FLAGS) $(DEBUG_FLAGS) $(BASE_FLAGS)
	ld -o $(BIN_DIR)/$(MAIN_PATH) $(BUILD_DIR)/$(MAIN_PATH).o

run:
	mkdir -p $(BUILD_DIR) $(BIN_DIR)
	nasm -o $(BUILD_DIR)/$(MAIN_PATH).o $(SRC_DIR)/$(MAIN_PATH).s \
		$(INCLUDE_FLAGS) $(DEBUG_FLAGS) $(BASE_FLAGS)
	ld -o $(BIN_DIR)/$(MAIN_PATH) $(BUILD_DIR)/$(MAIN_PATH).o
	./$(BIN_DIR)/$(MAIN_PATH)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

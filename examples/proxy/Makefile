ROOT_DIR = ../..
INCLUDE_DIR = $(ROOT_DIR)/include
LIB_DIR = $(ROOT_DIR)/lib
BUILD_DIR = build
BIN_DIR = bin
SRC_DIR = src

PROXY_PATH = proxy
SERVER1_PATH = server1
SERVER2_PATH = server2
SERVER3_PATH = server3

INCLUDE_DIRS = \
	$(shell find $(INCLUDE_DIR) -type d -printf '-I$(INCLUDE_DIR)/%P ')

LIB_DIRS = $(shell find $(LIB_DIR) -type d -printf '-I$(LIB_DIR)/%P ')

INCLUDE_FLAGS = $(INCLUDE_DIRS) $(LIB_DIRS)

DEBUG_FLAGS = -g
BASE_FLAGS = -felf64 -w+all

proxy:
	mkdir -p $(BUILD_DIR) $(BIN_DIR)
	nasm -o $(BUILD_DIR)/$(PROXY_PATH).o $(SRC_DIR)/$(PROXY_PATH).s \
		$(INCLUDE_FLAGS) $(DEBUG_FLAGS) $(BASE_FLAGS)
	ld -o $(BIN_DIR)/$(PROXY_PATH) $(BUILD_DIR)/$(PROXY_PATH).o
	nasm -o $(BUILD_DIR)/$(SERVER1_PATH).o $(SRC_DIR)/$(SERVER1_PATH).s \
		$(INCLUDE_FLAGS) $(DEBUG_FLAGS) $(BASE_FLAGS)
	ld -o $(BIN_DIR)/$(SERVER1_PATH) $(BUILD_DIR)/$(SERVER1_PATH).o
	nasm -o $(BUILD_DIR)/$(SERVER2_PATH).o $(SRC_DIR)/$(SERVER2_PATH).s \
		$(INCLUDE_FLAGS) $(DEBUG_FLAGS) $(BASE_FLAGS)
	ld -o $(BIN_DIR)/$(SERVER2_PATH) $(BUILD_DIR)/$(SERVER2_PATH).o
	nasm -o $(BUILD_DIR)/$(SERVER3_PATH).o $(SRC_DIR)/$(SERVER3_PATH).s \
		$(INCLUDE_FLAGS) $(DEBUG_FLAGS) $(BASE_FLAGS)
	ld -o $(BIN_DIR)/$(SERVER3_PATH) $(BUILD_DIR)/$(SERVER3_PATH).o

run-proxy:
	./$(BIN_DIR)/$(PROXY_PATH)

run-server1:
	./$(BIN_DIR)/$(SERVER1_PATH)

run-server2:
	./$(BIN_DIR)/$(SERVER2_PATH)

run-server3:
	./$(BIN_DIR)/$(SERVER3_PATH)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR) $(BIN_DIR)

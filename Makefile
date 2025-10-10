INCLUDE_DIR = include
LIB_DIR = lib
SRC_DIR = src
BIN_DIR = bin
BUILD_DIR = build

HERVE_PATH = herve

SERVER_PATH = examples/test/server
SERVER_PORT = 1337

ECHO_PATH = examples/echo/echo

REACT_PATH = examples/react/react

JSON_PATH = include/encoding/json_test

PROXY_PATH = examples/reverse_proxy/reverse_proxy
PROXY_SERVER_1_PATH = examples/reverse_proxy/server
PROXY_SERVER_2_PATH = examples/reverse_proxy/server2

INCLUDE_FILES = \
	$(shell find -type f -name '*.inc')

INCLUDE_DIRS = \
	$(shell find $(INCLUDE_DIR) -type d -printf '-I$(INCLUDE_DIR)/%P ')

LIB_DIRS = $(shell find $(LIB_DIR) -type d -printf '-I$(LIB_DIR)/%P ')

INCLUDE_FLAGS = $(INCLUDE_DIRS) $(LIB_DIRS)

DEBUG_FLAGS = -g
BASE_FLAGS = -felf64 -w+all

herve:
	mkdir -p build bin
	nasm -o $(BUILD_DIR)/$(HERVE_PATH).o $(SRC_DIR)/$(HERVE_PATH).s \
		$(BASE_FLAGS) $(INCLUDE_FLAGS) $(DEBUG_FLAGS)
	ld -o $(BIN_DIR)/$(HERVE_PATH) $(BUILD_DIR)/$(HERVE_PATH).o

herve-run:
	mkdir -p build bin
	nasm -o $(BUILD_DIR)/$(HERVE_PATH).o $(SRC_DIR)/$(HERVE_PATH).s \
		$(BASE_FLAGS) $(INCLUDE_FLAGS) $(DEBUG_FLAGS)
	ld -o $(BIN_DIR)/$(HERVE_PATH) $(BUILD_DIR)/$(HERVE_PATH).o
	./$(BIN_DIR)/$(HERVE_PATH)

server:
	nasm -o $(SERVER_PATH).o $(SERVER_PATH).s \
		$(BASE_FLAGS) $(INCLUDE_FLAGS) $(DEBUG_FLAGS)
	ld -o $(SERVER_PATH) $(SERVER_PATH).o

echo:
	nasm -o $(ECHO_PATH).o $(ECHO_PATH).s \
		$(BASE_FLAGS) $(INCLUDE_FLAGS) $(DEBUG_FLAGS)
	ld -o $(ECHO_PATH) $(ECHO_PATH).o

react:
	nasm -o $(REACT_PATH).o $(REACT_PATH).s \
		$(BASE_FLAGS) $(INCLUDE_FLAGS) $(DEBUG_FLAGS)
	ld -o $(REACT_PATH) $(REACT_PATH).o

proxy:
	nasm -o $(PROXY_PATH).o $(PROXY_PATH).s \
		$(BASE_FLAGS) $(INCLUDE_FLAGS) $(DEBUG_FLAGS)
	ld -o $(PROXY_PATH) $(PROXY_PATH).o
	nasm -o $(PROXY_SERVER_1_PATH).o $(PROXY_SERVER_1_PATH).s \
		$(BASE_FLAGS) $(INCLUDE_FLAGS) $(DEBUG_FLAGS)
	ld -o $(PROXY_SERVER_1_PATH) $(PROXY_SERVER_1_PATH).o
	nasm -o $(PROXY_SERVER_2_PATH).o $(PROXY_SERVER_2_PATH).s \
		$(BASE_FLAGS) $(INCLUDE_FLAGS) $(DEBUG_FLAGS)
	ld -o $(PROXY_SERVER_2_PATH) $(PROXY_SERVER_2_PATH).o

.PHONY: clean
clean:
	rm -f $(ECHO_PATH) $(ECHO_PATH).o
	rm -f $(TEST_PATH) $(TEST_PATH).o
	rm -f $(PROXY_PATH) $(PROXY_PATH).o
	rm -f $(PROXY_SERVER_1_PATH) $(PROXY_SERVER_1_PATH).o
	rm -f $(PROXY_SERVER_2_PATH) $(PROXY_SERVER_2_PATH).o


INC_DIR = inc
SERVER_DIR = server
MALLOC_DIR = malloc
UTILS_DIR = utils
CONFIG_DIR = config
BOEUF_DIR = boeuf
OS_DIR = os
LOGAN_DIR = logan
INCLUDES = $(shell find $(INC_DIR) -type f -name '*.inc')

INCLUDE_FLAGS = \
	-I$(INC_DIR) \
	-I$(INC_DIR)/$(SERVER_DIR) \
	-I$(INC_DIR)/$(UTILS_DIR) \
	-I$(INC_DIR)/$(MALLOC_DIR) \
	-I$(INC_DIR)/$(BOEUF_DIR) \
	-I$(INC_DIR)/$(OS_DIR) \
	-I$(INC_DIR)/$(LOGAN_DIR) \
	-I$(INC_DIR)/$(CONFIG_DIR)

DEBUG_FLAGS = -g
BASE_FLAGS = -felf64 -w+all

ifndef PROGRAM_NAME
$(error PROGRAM_NAME is not set. Please pass it on the command line, e.g., make PROGRAM_NAME=examples/echo/echo)
endif

all: $(INCLUDES) $(PROGRAM_NAME).s
	nasm -o $(PROGRAM_NAME).o $(PROGRAM_NAME).s \
		$(BASE_FLAGS) $(INCLUDE_FLAGS) $(DEBUG_FLAGS)
	ld -o $(PROGRAM_NAME) $(PROGRAM_NAME).o

.PHONY: clean
clean:
	rm -f $(PROGRAM_NAME) $(PROGRAM_NAME).o main main.o


INCLUDE_DIR = include
LIB_DIR = lib

HERVE_PATH = herve

PROJECT_PATH = <your-project-path>

INCLUDE_DIRS = \
	$(shell find $(INCLUDE_DIR) -type d -printf '-I$(INCLUDE_DIR)/%P ')

LIB_DIRS = $(shell find $(LIB_DIR) -type d -printf '-I$(LIB_DIR)/%P ')

SRC_DIRS = $(shell find $(SRC_DIR) -type d -printf '-I$(SRC_DIR)/%P ')

INCLUDE_FLAGS = $(INCLUDE_DIRS) $(LIB_DIRS) $(SRC_DIRS)

DEBUG_FLAGS = -g
BASE_FLAGS = -felf64 -w+all

all:
	nasm -o $(PATH_PATH).o $(PATH_PATH).s \
		$(BASE_FLAGS) $(INCLUDE_FLAGS) $(DEBUG_FLAGS)
	ld -o $(PATH_PATH) $(PATH_PATH).o


.PHONY: clean
clean:
	rm -f $(PATH_PATH) $(PATH_PATH).o


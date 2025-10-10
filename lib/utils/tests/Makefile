INCLUDE_DIR = ../../../include
LIB_DIR = ../..

ARRAY_PATH = array_test

INCLUDE_DIRS = \
	$(shell find $(INCLUDE_DIR) -type d -printf '-I$(INCLUDE_DIR)/%P ')

LIB_DIRS = $(shell find $(LIB_DIR) -type d -printf '-I$(LIB_DIR)/%P ')

INCLUDE_FLAGS = $(INCLUDE_DIRS) $(LIB_DIRS)

DEBUG_FLAGS = -g
BASE_FLAGS = -felf64 -w+all

array:
	nasm -o $(ARRAY_PATH).o $(ARRAY_PATH).s \
		$(BASE_FLAGS) $(INCLUDE_FLAGS) $(DEBUG_FLAGS)
	ld -o $(ARRAY_PATH) $(ARRAY_PATH).o
	./$(ARRAY_PATH)

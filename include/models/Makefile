INCLUDE_DIR = ..
LIB_DIR = ../../lib

MODELS_PATH = models_test

INCLUDE_DIRS = \
	$(shell find $(INCLUDE_DIR) -type d -printf '-I$(INCLUDE_DIR)/%P ')

LIB_DIRS = $(shell find $(LIB_DIR) -type d -printf '-I$(LIB_DIR)/%P ')

INCLUDE_FLAGS = $(INCLUDE_DIRS) $(LIB_DIRS)

DEBUG_FLAGS = -g
BASE_FLAGS = -felf64 -w+all

all:
	nasm -o $(MODELS_PATH).o $(MODELS_PATH).s \
		$(BASE_FLAGS) $(INCLUDE_FLAGS) $(DEBUG_FLAGS)
	ld -o $(MODELS_PATH) $(MODELS_PATH).o
	./$(MODELS_PATH)

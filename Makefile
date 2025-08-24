INCLUDE_DIR = include
LIB_DIR = lib

INCLUDE_FILES = \
	$(shell find -type f -name '*.inc')

INCLUDE_DIRS = \
	$(shell find $(INCLUDE_DIR) -type d -printf '-Iinclude/%P ')

LIB_DIRS = $(shell find $(LIB_DIR) -type d -printf '-Ilib/%P ')

INCLUDE_FLAGS = $(INCLUDE_DIRS) $(LIB_DIRS)

DEBUG_FLAGS = -g
BASE_FLAGS = -felf64 -w+all

ifndef PROGRAM_NAME
$(error PROGRAM_NAME is not set. Please pass it on the command line, e.g., make PROGRAM_NAME=examples/echo/echo)
endif

all: $(INCLUDE_FILES) $(PROGRAM_NAME).s
	nasm -o $(PROGRAM_NAME).o $(PROGRAM_NAME).s \
		$(BASE_FLAGS) $(INCLUDE_FLAGS) $(DEBUG_FLAGS)
	ld -o $(PROGRAM_NAME) $(PROGRAM_NAME).o

.PHONY: clean
clean:
	rm -f $(PROGRAM_NAME) $(PROGRAM_NAME).o main main.o


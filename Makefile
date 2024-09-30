INC_DIR = inc
INCLUDES = $(wildcard $(INC_DIR)/*.inc)

example: $(INCLUDES) examples/server.s
	nasm -f elf64 -o examples/server.o examples/server.s -g -w+all -I$(INC_DIR)/
	ld -o examples/server examples/server.o

test: test.c
	gcc -o test test.c
	./test


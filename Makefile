INC_DIR = inc
INCLUDES = $(wildcard $(INC_DIR)/*.inc)
INCLUDE_URL = https://github.com/bla-ce/unstack/releases/download/v1.0/unstack-v1.0.tar.gz

main: $(INCLUDES) examples/server.s
	nasm -f elf64 -o examples/server.o examples/server.s -g -w+all -I$(INC_DIR)/
	ld -o examples/server examples/server.o

test: test.s
	nasm -f elf64 -o test.o test.s -g
	ld -o test test.o
	./test


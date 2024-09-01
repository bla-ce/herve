INC_DIR = inc
INCLUDES = $(wildcard $(INC_DIR)/*.inc)

server: server.o
	ld -o server server.o

server.o: $(INCLUDES) server.s
	nasm -felf64 -o server.o server.s -g -w+all -I$(INC_DIR)/

run:
	nasm -felf64 -o server.o server.s -g -w+all -I$(INC_DIR)/
	ld -o server server.o
	./server

clean:
	rm -f server server.o

test: test.c
	gcc -o test test.c
	./test


INC_DIR = inc
INCLUDES = $(wildcard $(INC_DIR)/*.inc)

example: $(INCLUDES) examples/server.s
	nasm -felf64 -o examples/server.o examples/server.s -g -w+all -I$(INC_DIR)/
	ld -o examples/server examples/server.o
	./examples/server

prod:
	ld -o server server.o
	nasm -felf64 -o server.o server.s -g -w+all -I$(INC_DIR)/
	./server

test: test.c
	gcc -o test test.c
	./test

build:
	docker build -t byt-asm .
	docker run -d -p 1337:1337 --name byt-asm byt-asm

clean:
	docker stop byt-asm
	docker container prune
	docker image rm byt-asm


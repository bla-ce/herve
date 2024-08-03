server: server.o
	ld -o server server.o

server.o: server.s
	nasm -felf64 -o server.o server.s -g

test:
	gcc -o main main.c
	./main

run:
	nasm -felf64 -o server.o server.s -g
	ld -o server server.o
	./server

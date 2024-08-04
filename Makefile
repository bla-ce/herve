server: server.o
	ld -o server server.o

server.o: server.s
	nasm -felf64 -o server.o server.s -g -w+all

test:
	gcc -o test test.c
	./server

run:
	nasm -felf64 -o server.o server.s -g -w+all
	ld -o server server.o
	./server

clean:
	rm -f server server.o

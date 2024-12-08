INC_DIR = inc
INCLUDES = $(wildcard $(INC_DIR)/*.inc)
INCLUDE_URL = https://github.com/bla-ce/unstack/releases/download/v1.0/unstack-v1.0.tar.gz

main: $(INCLUDES) examples/server.s
	curl -L https://github.com/bla-ce/unstack/releases/download/v1.0/unstack-v1.0.tar.gz -o unstack-v1.0.tar.gz
	tar -xvf unstack-v1.0.tar.gz --strip-components=1 -C inc/ src/
	rm unstack-v1.0.tar.gz
	nasm -f elf64 -o examples/server.o examples/server.s -g -w+all -I$(INC_DIR)/
	ld -o examples/server examples/server.o
	rsync -av -e ssh --exclude='.git/' . nerdy@192.168.122.129:/home/nerdy/dev/assembly/bytasm

test: test.s
	nasm -f elf64 -o test.o test.s -g
	ld -o test test.o
	./test


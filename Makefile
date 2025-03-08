INC_DIR = inc
SERVER_DIR = server
MALLOC_DIR = malloc
UTILS_DIR = utils
INCLUDES = $(wildcard $(INC_DIR)/*.inc)
PROGRAM_NAME = examples/server

main: $(INCLUDES) $(PROGRAM_NAME).s
	nasm -f elf64 -o $(PROGRAM_NAME).o $(PROGRAM_NAME).s \
		-g -w+all  \
		-I$(INC_DIR)/ \
		-I$(INC_DIR)/$(SERVER_DIR) \
		-I$(INC_DIR)/$(UTILS_DIR) \
		-I$(INC_DIR)/$(MALLOC_DIR)
	ld -o $(PROGRAM_NAME) $(PROGRAM_NAME).o

test: main.s
	nasm -f elf64 -o main.o main.s -g -I$(INC_DIR) -w+all
	ld -o main main.o



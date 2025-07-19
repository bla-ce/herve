%include "ascii.inc"
%include "boeuf.inc"
%include "calloc.inc"
%include "client.inc"
%include "config.inc"
%include "constants.inc"
%include "context.inc"
%include "error.inc"
%include "file.inc"
%include "free.inc"
%include "hash_table.inc"
%include "headers.inc"
%include "logger.inc"
%include "malloc.inc"
%include "memset.inc"
%include "method.inc"
%include "middleware.inc"
%include "mmap.inc"
%include "realloc.inc"
%include "request.inc"
%include "response.inc"
%include "route.inc"
%include "select.inc"
%include "server.inc"
%include "socket.inc"
%include "status_code.inc"
%include "string.inc"
%include "syscall.inc"
%include "time.inc"

section .bss
section .data

HERVE_LOGO db  " _____________________________________________ ",LINE_FEED
           db  "|    _____   _____   _____   _____   _____    |",LINE_FEED
           db  "|   |  |  | |   __| | __  | |  |  | |   __|   |",LINE_FEED
           db  "|   |     | |   __| |    -| |  |  | |   __|   |",LINE_FEED
           db  "|   |__|__| |_____| |__|__|  \___/  |_____|   |",LINE_FEED
           db  "|_____________________________________________|",LINE_FEED, NULL_CHAR

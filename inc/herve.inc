; TODO: implement strcat
; TODO: clean up log functions
; TODO: set file as logger output
; TODO: ipv6 server with dual stack config ?
; TODO: handling multiple connections (select? concurrency? fork? poll?)
; TODO: fix date :)
; TODO: edge cases basic
; TODO: add stack usage to functions
; TODO: return date
; TODO: persist connection by default
; TODO: read parameters
; TODO: error management
; TODO: free ctx resources after connection
; TODO: timeout with alarm syscall
; TODO: update routes data structure faster than O(n)
; TODO: switch case implementation

%include "ascii.inc"
%include "base64.inc"
%include "basic.inc"
%include "calloc.inc"
%include "constants.inc"
%include "error.inc"
%include "file.inc"
%include "free.inc"
%include "headers.inc"
%include "logger.inc"
%include "malloc.inc"
%include "memset.inc"
%include "middleware.inc"
%include "mmap.inc"
%include "os.inc"
%include "request.inc"
%include "response.inc"
%include "server.inc"
%include "string.inc"
%include "time.inc"
%include "socket.inc"
%include "route.inc"
%include "method.inc"
%include "context.inc"
%include "client.inc"
%include "status_code.inc"
%include "syscall.inc"
%include "hash_table.inc"
%include "template.inc"
%include "redirect.inc"

section .bss
section .data


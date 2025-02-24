; TODO: return date
; TODO: use strndup for parsing
; TODO: persist connection by default
; TODO: error management
; TODO: free ctx resources after connection
; TODO: timeout with alarm syscall

%include "ascii.inc"
%include "calloc.inc"
%include "constants.inc"
%include "error.inc"
%include "file.inc"
%include "free.inc"
%include "headers.inc"
%include "log.inc"
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

section .bss
section .data
  CRLF  db CARRIAGE_RETURN, LINE_FEED, CARRIAGE_RETURN, LINE_FEED, NULL_CHAR



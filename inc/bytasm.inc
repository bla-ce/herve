; TODO: getter and setter methods 
; TODO: free ctx resources
; TODO: socket, server, client, engine?, response struct
; TODO: verify paddings and alignments
; TODO: timeout with alarm syscall
; TODO: abort request from middleware

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
%include "cookie.inc"
%include "socket.inc"
%include "route.inc"
%include "method.inc"
%include "context.inc"
%include "client.inc"

section .bss
  response     resq RESPONSE_MAX_LEN
  response_len resq 1

  ; response string
  response_status   resb STATUS_MAX_LEN
  response_proto    resb PROTO_MAX_LEN
  response_body     resb BODY_MAX_LEN

  ; response value
  response_status_code      resq 1
  response_proto_major      resd 1
  response_proto_minor      resd 1
  response_content_length   resw 1
  response_close            resb 1

  file      resb FILE_MAX_LEN
  file_len  resq 1

  duration resq 1

  routes_count resq 1

  callback_list resq MAX_ROUTES_COUNT
  callback      resq 1

  query_params  resb QUERY_MAX_LEN
  query         resb QUERY_MAX_VALUE_LEN

section .data
  enable  dw 1

  post_headers      db CARRIAGE_RETURN, LINE_FEED, CARRIAGE_RETURN, LINE_FEED, NULL_CHAR



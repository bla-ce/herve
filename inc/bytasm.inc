; TODO: malloc
; TODO: variable lifecycle
; TODO: file caching
; TODO: group routes
; TODO: doc and comments 
; TODO: errors
; TODO: mime type media
; TODO: add wildcard route
; TODO: GET request
; TODO: ANY request
; TODO: parse query string
; TODO: memory mapped file
; TODO: read file bigger then rdx
; TODO: set timeout
; TODO: session management
; TODO: block ip
; TODO: rate limit
; TODO: database integration
; TODO: url encode and decode
; TODO: graceful stop
; TODO: parse request headers
; TODO: templates
; TODO: add health_check
; TODO: JSON
; TODO: set/get cookie
; TODO: redirects
; TODO: public/private functions if possible
; TODO: read basic auth
; TODO: floating point for duration
; TODO: log in sec
; TODO: add https
; TODO: concurrency

%ifndef BYT_ASM_INC
%define BYT_ASM_INC

%include "constants.inc"
%include "middleware.inc"
%include "headers.inc"
%include "log.inc"
%include "mmap.inc"
%include "string.inc"
%include "server.inc"
%include "request.inc"
%include "response.inc"
%include "error.inc"
%include "file.inc"
%include "time.inc"
%include "os.inc"

section .bss
  port  resq 1

  request     resq 1
  request_len resq 1

  response     resq RESPONSE_MAX_LEN
  response_len resq 1

  ; request string
  request_method  resb METHOD_MAX_LEN
  request_url     resb URL_MAX_LEN
  request_proto   resb PROTO_MAX_LEN
  request_headers resb HEADERS_MAX_LEN
  request_body    resb BODY_MAX_LEN

  ; request value
  request_proto_major     resd 1
  request_proto_minor     resd 1
  request_content_length  resw 1
  request_close           resb 1
  request_host            resb HOST_MAX_LEN
  request_client          resq 1

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

  to_string_buf resb TO_STRING_MAX

  routes_count resq 1

  callback_list resq MAX_ROUTES_COUNT
  callback      resq 1

  query_params  resb QUERY_MAX_LEN
  query         resb QUERY_MAX_VALUE_LEN

section .data
  enable  dw 1

  server_sin_family  dw AF_INET
  server_sin_port    dw 0
  server_sin_addr    dd INADDR_ANY
  server_sa_zero     dq 0
  server_addrlen     equ $ - server_sin_family

  client_sin_family  dw AF_INET
  client_sin_port    dw 0
  client_sin_addr    dd 0
  client_sa_zero     dq 0
  client_addrlen     dq server_addrlen

  methods_list  db SPACE, "GET POST PUT DELETE HEAD OPTIONS PATCH TRACE CONNECT", SPACE ; add spaces for method checking
  methods_list_len equ $ - methods_list

  not_allowed_methods times methods_list_len db 0

  routes_list     times MAX_ROUTES_COUNT * STRUCT_ROUTE_LEN db 0
  routes_list_len equ $ - routes_list

  post_headers      db CARRIAGE_RETURN, LINE_FEED, CARRIAGE_RETURN, LINE_FEED, NULL_CHAR
  post_headers_len  equ $ - post_headers - 1

%endif


; TODO: redirects
; TODO: public/private functions if possible
; TODO: read basic auth
; TODO: add date and time to log
; TODO: floating point for duration
; TODO: log in sec
; TODO: add https
; TODO: doc and comments

%ifndef BYT_ASM_INC
%define BYT_ASM_INC

%include "constants.inc"
%include "log.inc"
%include "string.inc"
%include "server.inc"
%include "request.inc"
%include "response.inc"
%include "error.inc"
%include "file.inc"
%include "time.inc"

section .bss
  port  resq 1

  request     resb REQUEST_MAX_LEN
  request_len resq 1

  response     resb RESPONSE_MAX_LEN
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
  response_headers  resb HEADERS_MAX_LEN
  response_body     resb BODY_MAX_LEN

  dir_entries resb DIR_ENTRIES_LEN

  ; response value
  response_status_code      resq 1
  response_proto_major      resd 1
  response_proto_minor      resd 1
  response_content_length   resw 1
  response_close            resb 1

  file      resb FILE_MAX_LEN
  file_path resb FILE_PATH_LEN
  file_len  resq 1

  duration resq 1

  log_port resb LOG_PORT_LEN

  to_string_buf resb TO_STRING_MAX

  routes_count resq 1

  callback_list resq MAX_ROUTES_COUNT
  callback      resq 1

  query_params  resb QUERY_MAX_LEN
  query         resb QUERY_MAX_VALUE_LEN

  errno resb ERR_MAX_LEN

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

  active_connections  dq 0
  max_connections     dq DEFAULT_MAX_CONNECTIONS

  methods_list  db SPACE, "GET POST PUT DELETE HEAD OPTIONS PATCH TRACE CONNECT", SPACE ; add spaces for method checking
  methods_list_len equ $ - methods_list

  not_allowed_methods times methods_list_len db 0

  routes_list     times MAX_ROUTES_COUNT * STRUCT_ROUTE_LEN db 0
  routes_list_len equ $ - routes_list

  post_headers      db CARRIAGE_RETURN, LINE_FEED, CARRIAGE_RETURN, LINE_FEED, NULL_CHAR
  post_headers_len  equ $ - post_headers - 1

  ; log
  log_start     db " [LOG] "
  log_time      times LOG_TIME_SIZE db SPACE
  log_delim4    db " | "
  log_method    times METHOD_MAX_LEN db SPACE
  log_delim     db " | "
  log_url       times URL_MAX_LEN db SPACE
  log_delim2    db " | " 
  log_duration  times LONG_SIZE+2 db SPACE
  log_delim3    db " | "
  log_status    times STATUS_CODE_LEN db SPACE
  log_new_line  db LINE_FEED
  log_len       equ $ - log_start


  log_server_init_msg db " [DEBUG] Listening and serving HTTP on :"
  log_server_init_len equ $ - log_server_init_msg

  log_routes_available_msg db LINE_FEED, " [DEBUG] Routes available:", NULL_CHAR
  log_routes_available_len equ $ - log_routes_available_msg

  log_debug     db " [DEBUG] ", NULL_CHAR
  log_debug_len equ $ - log_debug

  ; error messages
  ERR_PORT_BOUNDS   db "[ERROR] port out of bounds", LINE_FEED, NULL_CHAR
  ERR_CREAT_SOCKET  db "[ERROR] failed to create socket", LINE_FEED, NULL_CHAR
  ERR_SET_SOCKOPT   db "[ERROR] failed to set socket option", LINE_FEED, NULL_CHAR
  ERR_BIND_SOCKET   db "[ERROR] failed to bind socket", LINE_FEED, NULL_CHAR
  ERR_LISTEN_SOCKET db "[ERROR] failed to listen to socket", LINE_FEED, NULL_CHAR

  ERR_METHOD_NOT_EXIST  db "[ERROR] method does not exist", LINE_FEED, NULL_CHAR
  ERR_METHOD_TOO_LONG   db "[ERROR] method too long", LINE_FEED, NULL_CHAR

  ERR_MALFORMED_REQUEST db " [ERROR] malformed request", NULL_CHAR

%endif


; TODO: errors (from building and running)
; TODO: save query parameters
; TODO: add increment value for strpos
; TODO: Clean refactor
; TODO: read basic auth
; TODO: check arguments from function
; TODO: add date and time to log
; TODO: response struct
; TODO: remove response message and construct them with response struct
; TODO: floating point for duration
; TODO: log in sec
; TODO: atoi
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

section .data
  enable  dw 1

  port  dq 0

  server_sin_family  dw AF_INET
  server_sin_port    dw 0
  server_sin_addr    dd INADDR_ANY
  server_sa_zero     dq 0
  server_addrlen     equ $ - server_sin_family

  client_sin_family  dw 0
  client_sin_port    dw 0
  client_sin_addr    dd 0
  client_sa_zero     dq 0
  client_addrlen     dq server_addrlen

  request     times REQUEST_MAX_LEN db 0
  request_len dq 0

  ; request string
  request_method          times METHOD_MAX_LEN db 0
  request_url             times URL_MAX_LEN db 0
  request_proto           times PROTO_MAX_LEN db 0
  request_headers         times HEADERS_MAX_LEN db 0
  request_body            times BODY_MAX_LEN db 0

  ; request value
  request_proto_major     dd 0
  request_proto_minor     dd 0
  request_content_length  dw 0
  request_close           db 0
  request_host            times HOST_MAX_LEN db 0
  request_client          dq 0

  ; response string
  response_status           times STATUS_MAX_LEN db 0
  response_proto            times PROTO_MAX_LEN db 0
  response_body             times BODY_MAX_LEN db 0

  ; response value
  response_status_code      dq 0
  response_proto_major      dd 0
  response_proto_minor      dd 0
  response_content_length   dw 0
  response_close            db 0

  file      times FILE_MAX_LEN db 0
  file_len  dq 0

  methods_list     db SPACE, "GET POST PUT DELETE HEAD OPTIONS PATCH TRACE CONNECT", SPACE ; add spaces for method checking
  methods_list_len equ $ - methods_list

  not_allowed_methods times methods_list_len db 0

  routes_list     times MAX_ROUTES_COUNT * STRUCT_ROUTE_LEN db 0
  routes_list_len equ $ - routes_list
  routes_count dq 0

  callback_list times MAX_ROUTES_COUNT dq 0
  callback      dq 0

  post_headers      db CARRIAGE_RETURN, LINE_FEED, CARRIAGE_RETURN, LINE_FEED, NULL_CHAR
  post_headers_len  equ $ - post_headers - 1

  ; responses
  response_200      db "HTTP/1.1 200 OK", CARRIAGE_RETURN, LINE_FEED 
                    db "Content-Type: text/html; charset=UTF-8", CARRIAGE_RETURN, LINE_FEED
                    db "Connection: close", CARRIAGE_RETURN, LINE_FEED
                    db CARRIAGE_RETURN, LINE_FEED
  response_200_len  equ $ - response_200

  response_400      db "HTTP/1.1 400 Bad Request", CARRIAGE_RETURN, LINE_FEED
                    db "Content-Type: text/html; charset=UTF-8", CARRIAGE_RETURN, LINE_FEED
                    db "Connection: close", CARRIAGE_RETURN, LINE_FEED
                    db CARRIAGE_RETURN, LINE_FEED
  response_400_len  equ $ - response_400

  response_401      db "HTTP/1.1 401 Unauthorized", CARRIAGE_RETURN, LINE_FEED
                    db "Content-Type: text/html; charset=UTF-8", CARRIAGE_RETURN, LINE_FEED
                    db "Connection: close", CARRIAGE_RETURN, LINE_FEED
                    db CARRIAGE_RETURN, LINE_FEED
  response_401_len  equ $ - response_401

  response_404      db "HTTP/1.1 404 Not Found", CARRIAGE_RETURN, LINE_FEED
                    db "Content-Type: text/html; charset=UTF-8", CARRIAGE_RETURN, LINE_FEED
                    db "Connection: close", CARRIAGE_RETURN, LINE_FEED
                    db CARRIAGE_RETURN, LINE_FEED
  response_404_len  equ $ - response_404

  response_405      db "HTTP/1.1 405 Method Not Allowed", CARRIAGE_RETURN, LINE_FEED
                    db "Content-Type: text/html; charset=UTF-8", CARRIAGE_RETURN, LINE_FEED
                    db "Connection: close", CARRIAGE_RETURN, LINE_FEED
                    db CARRIAGE_RETURN, LINE_FEED
  response_405_len  equ $ - response_405

  response_500      db "HTTP/1.1 500 Internal Server Error", CARRIAGE_RETURN, LINE_FEED
                    db "Content-Type: text/html; charset=UTF-8", CARRIAGE_RETURN, LINE_FEED
                    db "Connection: close", CARRIAGE_RETURN, LINE_FEED
                    db CARRIAGE_RETURN, LINE_FEED
  response_500_len  equ $ - response_500

  ; timers
  start_tv_sec  dq 0
  start_tv_nsec dq 0

  end_tv_sec  dq 0
  end_tv_nsec dq 0

  duration dq 0
  
  ; log
  log_start     db " [LOG] "
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

  log_port  times LOG_PORT_LEN db 0
  log_debug     db " [DEBUG] ", NULL_CHAR
  log_debug_len equ $ - log_debug

  to_string_buf times TO_STRING_MAX db 0

  ; error messages
  ERR_PORT_BOUNDS   db "[ERROR] port out of bounds", LINE_FEED, NULL_CHAR
  ERR_CREAT_SOCKET  db "[ERROR] failed to create socket", LINE_FEED, NULL_CHAR
  ERR_SET_SOCKOPT   db "[ERROR] failed to set socket option", LINE_FEED, NULL_CHAR
  ERR_BIND_SOCKET   db "[ERROR] failed to bind socket", LINE_FEED, NULL_CHAR
  ERR_LISTEN_SOCKET db "[ERROR] failed to listen to socket", LINE_FEED, NULL_CHAR

  ERR_METHOD_NOT_EXIST db "[ERROR] method does not exist", LINE_FEED, NULL_CHAR

%endif


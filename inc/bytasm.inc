; TODO: add date and time to log
; TODO; error handling
; TODO: add https
; TODO: basic auth
; TODO: add function to route
; TODO: read request body
; TODO: doc

%ifndef BYT_ASM_INC
%define BYT_ASM_INC

%include "constants.inc"
%include "log.inc"
%include "string.inc"
%include "server.inc"

section .data
  enable    dw 0

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

  method      times METHOD_MAX_LEN db 0
  method_len  dq 0

  route      times ROUTE_MAX_LEN db 0
  route_len  dq 0

  response      times RESPONSE_MAX_LEN db 0
  response_len  dq 0

  file      times FILE_MAX_LEN db 0
  file_len  dq 0

  methods_list     db "GET POST PUT DELETE HEAD OPTIONS PATCH TRACE CONNECT", SPACE ; add space for method checking
  methods_list_len equ $ - methods_list

  not_allowed_methods times methods_list_len db 0

  routes_list     times MAX_ROUTES_COUNT * STRUCT_ROUTE_LEN db 0
  routes_list_len dq 0

  http_status_code dw 0

  ; route
  root_route      db "/", NULL_CHAR

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

  ; error messages
  error_method_too_big      db "[ERROR] request method length exceeds allowed bounds", LINE_FEED
  error_method_too_big_len  equ $ - error_method_too_big

  error_route_too_big      db "[ERROR] request route length exceeds allowed bounds", LINE_FEED
  error_route_too_big_len  equ $ - error_route_too_big

  error_max_route      db "[ERROR] maximum number of routes reached", LINE_FEED
  error_max_route_len  equ $ - error_max_route

  error_creating_socket      db "[ERROR] failed to create socket", LINE_FEED
  error_creating_socket_len  equ $ - error_creating_socket

  error_binding      db "[ERROR] failed to bind socket", LINE_FEED
  error_binding_len  equ $ - error_binding

  error_listening      db "[ERROR] failed to listen to socket", LINE_FEED
  error_listening_len  equ $ - error_listening

  error_accepting      db "[ERROR] failed to accept connection", LINE_FEED
  error_accepting_len  equ $ - error_accepting

  error_setting_socket_option			db "[ERROR] failed to set socket option", LINE_FEED
  error_setting_socket_option_len	equ $ - error_setting_socket_option

  error_reading_request			db "[ERROR] failed to read request", LINE_FEED
  error_reading_request_len	equ $ - error_reading_request

  error_open			db "[ERROR] failed to open file", LINE_FEED
  error_open_len	equ $ - error_open

  error_read_file			db "[ERROR] failed to read file", LINE_FEED
  error_read_file_len	equ $ - error_read_file

  method_doesnt_exist      db "[ERROR] method doesn't exist", LINE_FEED
  method_doesnt_exist_len  equ $ - method_doesnt_exist

  ; timers
  start_tv_sec  dq 0
  start_tv_nsec dq 0

  end_tv_sec  dq 0
  end_tv_nsec dq 0

  duration dq 0

  ; log
  log_start     db LINE_FEED, " [LOG] "
  log_method    times METHOD_MAX_LEN db SPACE
  log_delim     db " | "
  log_route     times ROUTE_MAX_LEN db SPACE
  log_delim2    db " | " 
  log_duration  times LONG_SIZE+2 db SPACE
  log_delim3    db " | "
  log_status    times STATUS_CODE_LEN db SPACE
  log_delim4    db " | "
  log_file      times RESPONSE_MAX_LEN db SPACE
  log_len       equ $ - log_start

%endif


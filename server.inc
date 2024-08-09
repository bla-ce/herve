; TODO: add date and time to log
; TODO; error handling
; TODO: add https
; TODO: basic auth
; TODO: serve files
; TODO: add function to route
; TODO: read request body
; TODO: url parameters
; TODO: url anchors

%ifndef SERVER_INC
%define SERVER_INC

; constants
CLOCK_REALTIME  equ 0
STDOUT          equ 1
LONG_SIZE       equ 10

; socket constants
INADDR_ANY          equ 0
SOCK_STREAM         equ 1
SOL_SOCKET          equ 1
BACKLOG             equ 1
SO_REUSEADDR        equ 2
SO_REUSEPORT        equ 15
AF_INET             equ 2
REQUEST_MAX_LEN     equ 8*1024
MSG_TRUNC           equ 32
METHOD_MAX_LEN      equ 8
ROUTE_MAX_LEN       equ 32
MAX_ROUTES_COUNT    equ 10
STATUS_CODE_LEN     equ 3
RESPONSE_MAX_LEN    equ 128
STRUCT_ROUTE_LEN    equ ROUTE_MAX_LEN + METHOD_MAX_LEN + RESPONSE_MAX_LEN

; syscall values
SYS_WRITE         equ 1
SYS_CLOSE         equ 3
SYS_SOCKET        equ 41
SYS_ACCEPT        equ 43
SYS_SENDTO        equ 44
SYS_RECVFROM      equ 45
SYS_BIND          equ 49
SYS_LISTEN        equ 50
SYS_SETSOCKOPT    equ 54
SYS_EXIT          equ 60
SYS_CLOCK_GETTIME equ 228

; ascii
NULL_CHAR       equ 0
END_OF_TEXT     equ 3
LINE_FEED       equ 10
CARRIAGE_RETURN equ 13
SPACE           equ 32
NUMBER_SIGN     equ 35
HYPHEN          equ 45
DOT             equ 46
SLASH           equ 47
ZERO_CHAR       equ 48
QUESTION_MARK   equ 63
LOW_N_CHAR      equ 110
LOW_S_CHAR      equ 115

; exit code
SUCCESS_CODE equ 0
FAILURE_CODE equ -1

%macro strpos 4
  ; %1 -> string
  ; %2 -> string length
  ; %3 -> substring
  ; %4 -> substring length
  sub   rsp, 8 ; string index and substring index
  mov   qword [rsp], 0

  mov   rsi, %1
  mov   rax, %2
  mov   rdi, %3
  mov   rbx, %4

%%loop:
  mov   rcx, rbx
  cld
  rep   cmpsb 
  je    %%found

  add   qword [rsp], rbx
  sub   qword [rsp], rcx

  cmp   qword [rsp], rax
  jg    %%not_found

  add   rdi, rcx
  sub   rdi, rbx

  jmp   %%loop

%%found:
  mov   rax, qword [rsp]
  jmp   %%return

%%not_found:
  mov   rax, -1
  jmp   %%return
  
%%return:
  add   rsp, 8
%endmacro

%macro get_length 1
  ; %1 -> str
  mov   rsi, %1
  mov   rcx, 0

  cmp   rsi, 0
  je    %%return

%%loop:
  mov   al, NULL_CHAR
  cmp   byte [rsi], al
  je    %%return

  inc   rsi
  inc   rcx

  jmp   %%loop

%%return:
  mov   rax, rcx
%endmacro

%macro log_add_status 0
  lea   rsi, [log_status]
  add   rsi, STATUS_CODE_LEN
  dec   rsi

  mov   eax, dword [http_status_code]
  
%%loop:
  xor   edx, edx
  mov   ecx, 0xA              ; divisor, dividend in rax
  div   ecx                   ; quotient in rax and remainder in edx

  add   edx, ZERO_CHAR

  mov   byte [rsi], dl

  dec   esi

  cmp   eax, 0
  jne   %%loop
%endmacro

%macro log_add_duration 0
  sub   rsp, 8 ; duration_str length
  mov   qword [rsp], 0

  lea   rsi, [log_duration]
  add   rsi, LONG_SIZE

  ; print ns backwards
  mov   byte [rsi], LOW_S_CHAR
  dec   rsi
  mov   byte [rsi], LOW_N_CHAR
  dec   rsi

  mov   rax, qword [duration]
  
%%loop:
  xor   rdx, rdx
  mov   rcx, 0xA              ; divisor, dividend in rax
  div   rcx                   ; quotient in rax and remainder in edx

  add   rdx, ZERO_CHAR

  mov   byte [rsi], dl

  dec   rsi

  cmp   rax, 0
  jne   %%loop

  add   rsp, 8
  %endmacro

%macro htons 1
  ; %1 -> port
  mov   eax, %1
  xchg  al, ah ; bswap 16-bit registers
%endmacro

%macro extract_method 2
  ; %1 -> request pointer
  ; %2 -> request length
  sub   rsp, 8 ; current inde[x
  mov   qword [rsp], 0

  mov   rsi, %1
  mov   rcx, %2
  xor   rax, rax

%%loop:
  lodsb
  cmp   al, SPACE
  je    %%return

  inc   qword [rsp]

  cmp   qword [rsp], rcx
  jl    %%loop

  ; bad request
  mov   rax, -1
  jmp   %%skip

%%return:
  dec   rsi
  mov   rax, qword [rsp]
  mov   qword [method_len], rax

  sub   rsi, rax
  lea   rdi, [method]
  mov   rcx, rax
  rep   movsb

%%skip:
  add   rsp, 8 
%endmacro

%macro check_method 2
  ; %1 -> method
  ; %2 -> method length
  sub   rsp, 16 

  lea   rsi, [error_method_too_big]
  mov   rdx, error_method_too_big_len

  mov   rcx, %2
  cmp   rcx, METHOD_MAX_LEN
  jg    error

  mov   qword [rsp], 0 ; string index
  mov   qword [rsp+8], 0 ; substring index

  mov   rsi, %1
  xor   rax, rax
  lea   rdi, [methods_list]
  xor   rbx, rbx

%%loop:
  cmp   qword [rsp], methods_list_len
  je    method_not_allowed
  
  cld
  cmpsb
  jne   %%mismatch

  inc   qword [rsp]
  inc   qword [rsp+8]

  cmp   rcx, qword [rsp+8]
  je    method_exists
  jmp   %%loop

%%mismatch:
  dec   rsi
  sub   rsi, qword [rsp+8]
  mov   qword [rsp+8], 0

  inc   qword [rsp]

  jmp   %%loop
  
method_not_allowed:
  mov   rax, -1
  jmp   %%return

method_exists:
  ; make sure we checked the entire method
  mov   al, byte [rdi]
  cmp   al, SPACE
  jne   method_not_allowed

  ; check if the method is allowed
  mov   rsi, methods_list
  sub   rdi, rsi
  mov   rbx, rdi
  sub   rbx, %2

  lea   rsi, %1
  lea   rdi, [not_allowed_methods]
  add   rdi, rbx

  mov   rcx, %2
  rep   cmpsb
  je    method_not_allowed

  mov   rax, qword [rsp]

%%return:
  add   rsp, 16
%endmacro

%macro check_route 2
  ; %1 -> route
  ; %2 -> route length
  sub   rsp, 72 

  lea   rsi, [error_route_too_big]
  mov   rdx, error_route_too_big_len

  mov   rcx, %2
  cmp   rcx, ROUTE_MAX_LEN
  jg    error

  mov   qword [rsp], 0 ; string index
  mov   qword [rsp+8], 0 ; substring index

  ; check if last char is a slash
  mov   rdi, %1
  add   rdi, rcx
  dec   rdi
  lea   rsi, [rdi]
  lodsb
  sub   rsi, rcx

  cmp   rcx, 1
  je    %%jump

  mov   rbx, rcx
  dec   rbx

  cmp   al, SLASH
  cmove rcx, rbx

%%jump:
  xor   rax, rax
  lea   rdi, [routes_list]
  xor   rbx, rbx

%%loop:
  mov   rax, [routes_list_len]
  cmp   qword [rsp], rax
  je    route_not_found
  
  cld
  cmpsb
  jne   %%mismatch

  inc   qword [rsp+8]

  cmp   rcx, qword [rsp+8]
  je    route_found
  jmp   %%loop

%%mismatch:
  dec   rsi
  dec   rdi

  sub   rsi, qword [rsp+8]
  sub   rdi, qword [rsp+8]
  add   rdi, STRUCT_ROUTE_LEN

  mov   qword [rsp+8], 0

  inc   qword [rsp]

  jmp   %%loop
  
route_not_found:
  mov   rax, -1
  jmp   %%return

route_found:
  mov   al, byte [rdi]
  cmp   al, NULL_CHAR
  jne   route_not_found

  ; test method
  sub   rdi, rcx
  add   rdi, ROUTE_MAX_LEN

  lea   rsi, [method]

  cld
  mov   rcx, qword [method_len]
  rep   cmpsb
  jne   route_not_found

  mov   rax, qword [rsp]

%%return:
  add   rsp, 72
%endmacro

%macro extract_route 2
  ; %1 -> request pointer
  ; %2 -> request length
  sub   rsp, 8 ; current index
  mov   qword [rsp], 0

  mov   rsi, %1
  mov   rcx, %2
  xor   rax, rax

%%loop:
  cmp   qword [rsp], ROUTE_MAX_LEN
  jge   %%bad_request

  lodsb
  cmp   al, SPACE
  je    %%return

  cmp   al, QUESTION_MARK
  je    %%return

  cmp   al, NUMBER_SIGN
  je    %%return

  inc   qword [rsp]

  cmp   qword [rsp], rcx
  jl    %%loop

%%bad_request:
  ; bad request
  mov   rax, -1
  jmp   %%skip

%%return:
  dec   rsi
  mov   rax, qword [rsp]
  mov   qword [route_len], rax

  sub   rsi, rax
  lea   rdi, [route]
  mov   rcx, rax
  rep   movsb

%%skip:
  add   rsp, 8 
%endmacro

%macro disallow_method 2
  ; %1 -> method
  ; %2 -> method_len
  mov   rsi, %1

  mov   rbx, %2
  mov   [method_len], rbx

  lea   rdi, [method]
  mov   rcx, rbx
  rep   movsb

  push  rbx

  dec   qword [method_len] ; remove null_char 
  strpos methods_list, methods_list_len, method, [method_len]

  lea   rsi, [method_doesnt_exist]
  mov   rdi, method_doesnt_exist_len
  cmp   rax, 0
  jl    error

  pop   rbx

  lea   rdi, [not_allowed_methods]
  add   rdi, rax

  lea   rsi, [method]
  mov   rcx, rbx
  rep   movsb

  xor   rax, rax
%endmacro

%macro add_route 3
  ; %1 -> method
  ; %2 -> route
  ; %3 -> response
  sub   rsp, 24
  mov   qword [rsp], 0 ; route_len
  mov   qword [rsp+8], 0 ; method_len

  lea   rsi, [error_route_too_big]
  mov   rdx, error_route_too_big_len

  mov   rcx, ROUTE_MAX_LEN

  ; method_length
  get_length %1
  mov   qword [rsp], rax

  ; route_length
  get_length %2

  lea   rsi, [error_route_too_big]
  mov   rdx, error_route_too_big_len

  cmp   rax, ROUTE_MAX_LEN
  jg    error

  mov   qword [rsp+8], rax

  ; response_length
  get_length %3
  mov   qword [rsp+16], rax

  mov   rcx, qword [rsp+8]
  mov   rsi, %2

  lea   rdi, [routes_list]

  mov   rax, qword [routes_list_len]
  mov   rbx, STRUCT_ROUTE_LEN
  mul   rbx

  add   rdi, rax
  rep   movsb

  mov   rax, ROUTE_MAX_LEN
  sub   rax, qword [rsp+8]
  add   rdi, rax

  mov   rsi, %1
  mov   rcx, qword [rsp]
  rep   movsb

  cmp   qword [rsp+16], 0
  jle   %%continue 

  mov   rax, METHOD_MAX_LEN
  sub   rax, qword [rsp]
  add   rdi, rax

  mov   rsi, %3
  mov   rcx, qword [rsp+16]
  rep   movsb

%%continue:
  inc   qword [routes_list_len]

  lea   rsi, [error_max_route]
  mov   rdx, error_max_route_len

  mov   rax, qword [routes_list_len]
  cmp   rax, MAX_ROUTES_COUNT
  jg    error

  add   rsp, 24
%endmacro

%macro server_init 1
  sub   rsp, 64
  mov   qword [rsp], 0  ; sockfd
  ; create socket
  mov   rax, SYS_SOCKET
  mov   rdi, AF_INET
  mov   rsi, SOCK_STREAM
  mov   rdx, 0
  syscall

  lea   rsi, [error_creating_socket]
  mov   rdx, error_creating_socket_len

  cmp   rax, 0
  jl    error

  mov   qword [rsp], rax

  ; set socket options
  mov   rax, SYS_SETSOCKOPT
  mov   rdi, qword [rsp]
  mov   rsi, SOL_SOCKET
  mov   rdx, SO_REUSEPORT
  mov   r10, enable
  mov   r8, 4 
  syscall

  lea   rsi, [error_setting_socket_option]
  mov   rdx, error_setting_socket_option_len

  cmp   rax, 0
  jl    error

  mov   rax, SYS_SETSOCKOPT
  mov   rdi, qword [rsp]
  mov   rsi, SOL_SOCKET
  mov   rdx, SO_REUSEADDR
  mov   r10, enable
  mov   r8, 4 
  syscall

  lea   rsi, [error_setting_socket_option]
  mov   rdx, error_setting_socket_option_len

  cmp   rax, 0
  jl    error

  add_route http_get, root_route, ""

  ; bind socket
  xor   rax, rax
  htons %1
  mov   dword [server_sin_port], eax
  mov   rax, SYS_BIND
  mov   rdi, qword [rsp]
  lea   rsi, [server_sin_family]
  mov   rdx, server_addrlen
  syscall

  lea   rsi, [error_binding]
  mov   rdx, error_binding_len

  cmp   rax, 0
  jl    error

  ; listen socket
  mov   rax, SYS_LISTEN
  mov   rdi, qword [rsp]
  mov   rsi, BACKLOG
  syscall

  lea   rsi, [error_listening]
  mov   rdx, error_listening_len

  cmp   rax, 0
  jl    error

  mov   rax, qword [rsp]
  add   rsp, 64
%endmacro

%macro run_server 1
  ; %1 -> sockfd
  sub   rsp, 128
  mov   rax, %1
  mov   qword [rsp], rax
  mov   qword [rsp+64], 0 ; client fd

%%loop:
  ; set end timer
  mov   rax, SYS_CLOCK_GETTIME
  mov   rdi, CLOCK_REALTIME
  lea   rsi, [end_tv_sec]
  syscall

  cmp   qword [start_tv_sec], 0
  je    timer_not_defined

  mov   rax, qword [end_tv_nsec]
  sub   rax, qword [start_tv_nsec]

  mov   qword [duration], rax

print_log:
  mov   rcx, METHOD_MAX_LEN
  mov   rax, SPACE
  lea   rdi, [log_method]
  rep   stosb

  mov   rcx, ROUTE_MAX_LEN
  mov   rax, SPACE
  lea   rdi, [log_route]
  rep   stosb

  mov   rcx, LONG_SIZE
  add   rcx, 2
  mov   rax, SPACE
  lea   rdi, [log_duration]
  rep   stosb

  lea   rsi, [method]
  lea   rdi, [log_method]
  mov   rcx, [method_len] 
  rep   movsb

  lea   rsi, [route]

  mov   al, byte [rsi]
  cmp   al, NULL_CHAR
  je    jump_log_route

  lea   rdi, [log_route]
  mov   rcx, [route_len]
  rep   movsb

jump_log_route:

  log_add_duration

  log_add_status

  mov   rax, SYS_WRITE
  mov   rdi, STDOUT
  lea   rsi, [log_start]
  mov   rdx, log_len
  syscall

timer_not_defined:
  ; clear route and method
  mov   rcx, ROUTE_MAX_LEN
  xor   rax, rax
  lea   rdi, [route]
  rep   stosb

  mov   rcx, METHOD_MAX_LEN
  xor   rax, rax
  lea   rdi, [method]
  rep   stosb

  ; accept connection
  mov   rax, SYS_ACCEPT
  mov   rdi, qword [rsp]
  lea   rsi, [client_sin_family]
  lea   rdx, [client_addrlen]
  syscall

  lea   rsi, [error_accepting]
  mov   rdx, error_accepting_len

  cmp   rax, 0
  jl    error

  mov   qword [rsp+64], rax

  ; set start timer
  mov   rax, SYS_CLOCK_GETTIME
  mov   rdi, CLOCK_REALTIME
  lea   rsi, [start_tv_sec]
  syscall

  ; receive client request
  mov   rax, SYS_RECVFROM
  mov   rdi, qword [rsp+64]
  lea   rsi, [request]
  mov   rdx, REQUEST_MAX_LEN
  xor   r10, r10
  xor   r9, r9
  xor   r8, r8
  syscall

  lea   rsi, [error_reading_request]
  mov   rdx, error_reading_request_len

  cmp   rax, 0
  jl    error

  mov   qword [request_len], rax

  ; extract method
  extract_method request, [request_len] 
  cmp   rax, 0
  jge   move_to_check_method

  mov   rax, SYS_WRITE
  mov   rdi, qword [rsp+64]
  mov   rsi, response_400
  mov   rdx, response_400_len
  syscall

  mov   dword [http_status_code], 400

  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp+64] 
  syscall

  jmp   %%loop

move_to_check_method:  
  ; check if method is allowed
  check_method method, [method_len]
  cmp   rax, 0
  jge   move_to_route

  mov   rax, SYS_WRITE
  mov   rdi, qword [rsp+64]
  mov   rsi, response_405
  mov   rdx, response_405_len
  syscall

  mov   dword [http_status_code], 405

  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp+64] 
  syscall

  jmp   %%loop

move_to_route:
  lea   rsi, [request]
  add   rsi, [method_len] ; move to route (add one for the space)
  inc   rsi
  mov   rcx, [request_len]
  sub   rcx, [method_len]
  dec   rcx

  ; extract route
  extract_route rsi, rcx 
  cmp   rax, 0
  jge   test_route

  mov   rax, SYS_WRITE
  mov   rdi, qword [rsp+64]
  mov   rsi, response_400
  mov   rdx, response_400_len
  syscall

  mov   dword [http_status_code], 400

  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp+64] 
  syscall

  jmp   %%loop

test_route:
  check_route route, [route_len]
  cmp   rax, 0
  jge   move_to_rest_of_request

  mov   rax, SYS_WRITE
  mov   rdi, qword [rsp+64]
  mov   rsi, response_404
  mov   rdx, response_404_len
  syscall

  mov   dword [http_status_code], 404

  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp+64] 
  syscall

  jmp   %%loop

move_to_rest_of_request:
  lea   rsi, [request]
  add   rsi, [route_len] ; move to rest of request (add one for the space)
  inc   rsi
  mov   rcx, [request_len]
  sub   rcx, [route_len]
  dec   rcx

  ; inside rax is the index of the route inside routes_list 
  mov   rbx, STRUCT_ROUTE_LEN
  mul   rbx

  add   rax, ROUTE_MAX_LEN
  add   rax, METHOD_MAX_LEN

  lea   rsi, [routes_list]
  add   rsi, rax

  lea   rdi, [response]
  mov   rcx, RESPONSE_MAX_LEN
  rep   movsb

  get_length response
  mov   qword [response_len], rax

  mov   dword [http_status_code], 200

  mov   rax, SYS_WRITE
  mov   rdi, qword [rsp+64]
  mov   rsi, response_200
  mov   rdx, response_200_len
  syscall

  mov   rax, SYS_WRITE
  mov   rdi, qword [rsp+64]
  lea   rsi, [response]
  mov   rdx, qword [response_len]
  syscall

  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp+64] 
  syscall

  jmp   %%loop
%endmacro

%macro shutdown 0
  ; close sockets
  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp] 
  syscall

  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp+64] 
  syscall
%endmacro

error:
  ; write error message
  mov   rax, SYS_WRITE
  mov   rdi, STDOUT
  syscall
 
  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp+64] 
  syscall

  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp]
  syscall

  mov   rax, SYS_EXIT
  mov   rdi, FAILURE_CODE
  syscall

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

  route      times ROUTE_MAX_LEN + METHOD_MAX_LEN db 0
  route_len  dq 0

  response      times RESPONSE_MAX_LEN db 0
  response_len  dq 0

  methods_list     db "GET POST PUT DELETE HEAD OPTIONS PATCH TRACE CONNECT", SPACE ; add space for method checking
  methods_list_len equ $ - methods_list

  not_allowed_methods times methods_list_len db 0

  routes_list     times MAX_ROUTES_COUNT * STRUCT_ROUTE_LEN db 0
  routes_list_len dq 0

  ; methods
  http_get      db "GET", NULL_CHAR
  http_get_len  equ $ - http_get

  http_post     db "POST", NULL_CHAR
  http_post_len equ $ - http_post

  http_put      db "PUT", NULL_CHAR
  http_put_len  equ $ - http_put

  http_delete   db "DELETE", NULL_CHAR
  http_delete_len equ $ - http_delete

  http_head     db "HEAD", NULL_CHAR
  http_head_len equ $ - http_head

  http_options  db "OPTIONS", NULL_CHAR
  http_options_len equ $ - http_options

  http_patch    db "PATCH", NULL_CHAR
  http_patch_len equ $ - http_patch

  http_trace    db "TRACE", NULL_CHAR
  http_trace_len equ $ - http_trace

  http_connect  db "CONNECT", NULL_CHAR
  http_connect_len equ $ - http_connect

  http_status_code dw 0

  ; route
  root_route      db "/", NULL_CHAR
  root_route_len  equ $ - root_route

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

  response_405      db "HTTP/1.1 405 Method Not Allowed", CARRIAGE_RETURN, LINE_FEED
                    db "Content-Type: text/html; charset=UTF-8", CARRIAGE_RETURN, LINE_FEED
                    db "Connection: close", CARRIAGE_RETURN, LINE_FEED
                    db CARRIAGE_RETURN, LINE_FEED
  response_405_len  equ $ - response_405

  response_404      db "HTTP/1.1 404 Not Found", CARRIAGE_RETURN, LINE_FEED
                    db "Content-Type: text/html; charset=UTF-8", CARRIAGE_RETURN, LINE_FEED
                    db "Connection: close", CARRIAGE_RETURN, LINE_FEED
                    db CARRIAGE_RETURN, LINE_FEED
  response_404_len  equ $ - response_404

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
  log_end       db " | "
  log_len       equ $ - log_start

%endif


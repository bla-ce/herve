global  _start

; constants
STDOUT  equ 2

; socket constants
INADDR_ANY        equ 0
SOCK_STREAM       equ 1
SOL_SOCKET        equ 1
BACKLOG           equ 1
SO_REUSEADDR      equ 2
SO_REUSEPORT      equ 15
AF_INET           equ 2
PORT              equ 14597 
REQUEST_MAX_LEN   equ 8*1024
MSG_TRUNC         equ 32
METHOD_MAX_LEN    equ 8
ROUTE_MAX_LEN     equ 32
MAX_ROUTES_COUNT  equ 10

; syscall values
SYS_WRITE       equ 1
SYS_CLOSE       equ 3
SYS_SOCKET      equ 41
SYS_ACCEPT      equ 43
SYS_SENDTO      equ 44
SYS_RECVFROM    equ 45
SYS_BIND        equ 49
SYS_LISTEN      equ 50
SYS_SETSOCKOPT  equ 54
SYS_EXIT        equ 60

; ascii
NULL_CHAR       equ 0
LINE_FEED       equ 10
CARRIAGE_RETURN equ 13
SPACE           equ 32

; exit code
SUCCESS_CODE equ 0
FAILURE_CODE equ -1

%macro funcall2 3
  mov   rax, %2
  mov   rdi, %3
  call  %1
%endmacro

extract_method:
  ; rax -> request pointer
  ; rdi -> request length
  push  rbp
  mov   rbp, rsp

  sub   rsp, 8 ; current index
  mov   qword [rsp], 0

  lea   rsi, [rax]
  mov   rcx, rdi
  xor   rax, rax

next_method_char:
  mov   al, byte [rsi]
  cmp   al, SPACE
  je    return_method

  inc   rsi
  inc   qword [rsp]

  cmp   qword [rsp], rcx
  jl    next_method_char

  ; bad request
  mov   rax, -1
  add   rsp, 8
  pop   rbp
  ret

return_method:
  mov   rax, qword [rsp]
  mov   qword [method_len], rax

  sub   rsi, rax
  lea   rdi, [method]
  mov   rcx, rax
  rep   movsb

  add   rsp, 8 
  pop   rbp
  ret

check_method:
  ; rax -> method
  ; rdi -> method length
  push  rbp
  mov   rbp, rsp
  sub   rsp, 16 

  cmp   rdi, METHOD_MAX_LEN
  jg    error

  mov   qword [rsp], 0 ; string index
  mov   qword [rsp+8], 0 ; substring index
  
  mov   rcx, rdi

  lea   rsi, [rax]
  xor   rax, rax
  lea   rdi, [methods_list]
  xor   rbx, rbx

check_next_char:
  cmp   qword [rsp], methods_list_len
  je    method_not_allowed
  
  mov   al, byte [rsi]
  mov   bl, byte [rdi]
  cmp   al, bl
  jne   char_mismatch

  inc   qword [rsp]
  inc   qword [rsp+8]
  inc   rsi
  inc   rdi

  cmp   rcx, qword [rsp+8]
  je    method_allowed
  jmp   check_next_char

char_mismatch:
  sub   rsi, qword [rsp+8]
  mov   qword [rsp+8], 0

  inc   qword [rsp]
  inc   rdi

  jmp   check_next_char
  
method_not_allowed:
  mov   rax, -1
  jmp   cleanup

method_allowed:
  mov   rax, qword [rsp]

cleanup:
  add   rsp, 16
  pop   rbp
  ret

check_route:
  ; rax -> route
  ; rdi -> route length
  push  rbp
  mov   rbp, rsp
  sub   rsp, 72 

  cmp   rdi, ROUTE_MAX_LEN
  jg    error

  mov   qword [rsp], 0 ; string index
  mov   qword [rsp+8], 0 ; substring index
  
  mov   rcx, rdi

  lea   rsi, [rax]
  xor   rax, rax
  lea   rdi, [routes_list]
  xor   rbx, rbx

check_next_route_char:
  mov   rax, [routes_list_len]
  mov   rbx, ROUTE_MAX_LEN
  mul   rbx

  cmp   qword [rsp], rax
  je    route_not_found
  
  mov   al, byte [rsi]
  mov   bl, byte [rdi]
  cmp   al, bl
  jne   route_char_mismatch

  inc   qword [rsp]
  inc   qword [rsp+8]
  inc   rsi
  inc   rdi

  cmp   rcx, qword [rsp+8]
  je    route_found
  jmp   check_next_route_char

route_char_mismatch:
  sub   rsi, qword [rsp+8]
  sub   rdi, qword [rsp+8]
  add   rdi, ROUTE_MAX_LEN

  mov   qword [rsp+8], 0

  inc   qword [rsp]

  jmp   check_next_route_char
  
route_not_found:
  mov   rax, -1
  jmp   route_cleanup

route_found:
  mov   rax, qword [rsp]

route_cleanup:
  add   rsp, 72
  pop   rbp
  ret

extract_route:
  ; rax -> request pointer
  ; rdi -> request length
  push  rbp
  mov   rbp, rsp

  sub   rsp, 8 ; current index
  mov   qword [rsp], 0

  lea   rsi, [rax]
  mov   rcx, rdi
  xor   rax, rax

next_route_char:
  mov   al, byte [rsi]
  cmp   al, SPACE
  je    return_route

  inc   rsi
  inc   qword [rsp]

  cmp   qword [rsp], rcx
  jl    next_route_char

  ; bad request
  mov   rax, -1
  add   rsp, 8
  pop   rbp
  ret

return_route:
  mov   rax, qword [rsp]
  mov   qword [route_len], rax

  sub   rsi, rax
  lea   rdi, [route]
  mov   rcx, rax
  rep   movsb

  add   rsp, 8 
  pop   rbp
  ret

add_route:
  ; rax -> route
  ; rdi -> route length
  push  rbp

  cmp   rdi, ROUTE_MAX_LEN
  jg    error

  mov   rcx, rdi

  lea   rsi, [rax]
  lea   rdi, [routes_list]

  mov   rax, qword [routes_list_len]

  mov   rbx, ROUTE_MAX_LEN
  mul   rbx

  add   rdi, rax
  rep   movsb

  inc   qword [routes_list_len]

  mov   rax, qword [routes_list_len]
  cmp   rax, MAX_ROUTES_COUNT
  jg    error

  pop   rbp
  ret

section .text
_start:
  ; create socket
  mov   rax, SYS_SOCKET
  mov   rdi, AF_INET
  mov   rsi, SOCK_STREAM
  mov   rdx, 0
  syscall

  cmp   rax, 0
  jl    error

  mov   qword [sockfd], rax

  ; set socket options
  mov   rax, SYS_SETSOCKOPT
  mov   rdi, [sockfd]
  mov   rsi, SOL_SOCKET
  mov   rdx, SO_REUSEPORT
  mov   r10, enable
  mov   r8, 4 
  syscall

  cmp   rax, 0
  jl    error

  mov   rax, SYS_SETSOCKOPT
  mov   rdi, [sockfd]
  mov   rsi, SOL_SOCKET
  mov   rdx, SO_REUSEADDR
  mov   r10, enable
  mov   r8, 4 
  syscall

  cmp   rax, 0
  jl    error

  ; bind socket
  mov   rax, SYS_BIND
  mov   rdi, [sockfd]
  lea   rsi, [server_sin_family]
  mov   rdx, server_addrlen
  syscall

  cmp   rax, 0
  jl    error

  ; listen socket
  mov   rax, SYS_LISTEN
  mov   rdi, [sockfd]
  mov   rsi, BACKLOG
  syscall

  cmp   rax, 0
  jl    error

  ; accept connection
  mov   rax, SYS_ACCEPT
  mov   rdi, [sockfd]
  lea   rsi, [client_sin_family]
  lea   rdx, [client_addrlen]
  syscall

  cmp   rax, 0
  jl    error

  mov   [clientfd], rax

  ; test add route 
  funcall2 add_route, test_route_1, test_route_1_len
  funcall2 add_route, test_route_2, test_route_2_len
  funcall2 add_route, test_route_3, test_route_3_len

  ; receive client request
  mov   rax, SYS_RECVFROM
  mov   rdi, [clientfd]
  lea   rsi, [request]
  mov   rdx, REQUEST_MAX_LEN
  xor   r10, r10
  xor   r9, r9
  xor   r8, r8
  syscall

  cmp   rax, 0
  jl    error

  mov   qword [request_len], rax

  ; extract method
  funcall2 extract_method, request, [request_len] 
  cmp   rax, 0
  jge   move_to_check_method

  mov   rax, SYS_WRITE
  mov   rdi, [clientfd]
  mov   rsi, response_400
  mov   rdx, response_400_len
  syscall

  jmp   exit

move_to_check_method:  
  ; check if method is allowed
  funcall2 check_method, method, [method_len]
  cmp   rax, 0
  jge   move_to_route

  mov   rax, SYS_WRITE
  mov   rdi, [clientfd]
  mov   rsi, response_405
  mov   rdx, response_405_len
  syscall

  jmp   exit

move_to_route:
  lea   rsi, [request]
  add   rsi, [method_len] ; move to route (add one for the space)
  inc   rsi
  mov   rcx, [request_len]
  sub   rcx, [method_len]
  dec   rcx

  ; extract route
  funcall2 extract_route, rsi, rcx 
  cmp   rax, 0
  jge   test_route

  mov   rax, SYS_WRITE
  mov   rdi, [clientfd]
  mov   rsi, response_400
  mov   rdx, response_400_len
  syscall

test_route:
  funcall2 check_route, route, [route_len]
  cmp   rax, 0
  jge   move_to_rest_of_request

  mov   rax, SYS_WRITE
  mov   rdi, [clientfd]
  mov   rsi, response_404
  mov   rdx, response_404_len
  syscall

  jmp   exit

move_to_rest_of_request:
  lea   rsi, [request]
  add   rsi, [route_len] ; move to rest of request (add one for the space)
  inc   rsi
  mov   rcx, [request_len]
  sub   rcx, [route_len]
  dec   rcx

  mov   rax, SYS_WRITE
  mov   rdi, [clientfd]
  mov   rsi, response_200
  mov   rdx, response_200_len
  syscall

exit:
  ; close sockets
  mov   rax, SYS_CLOSE
  mov   rdi, [sockfd] 
  syscall
  mov   rax, SYS_CLOSE
  mov   rdi, [clientfd] 
  syscall

  ; exit
  mov   rax, SYS_EXIT
  mov   rdi, SUCCESS_CODE
  syscall

send_400:
  mov   rax, SYS_WRITE
  mov   rdi, [clientfd]
  mov   rsi, response_400
  mov   rdx, response_400_len
  syscall

  mov   qword [clientfd], 0

error:
  ; close sockets
  cmp   qword [clientfd], 0
  jg    send_400
  
  mov   rax, SYS_CLOSE
  mov   rdi, [clientfd] 
  syscall

  mov   rax, SYS_CLOSE
  mov   rdi, [sockfd] 
  syscall

  mov   rax, SYS_EXIT
  mov   rdi, FAILURE_CODE
  syscall

section .data
  sockfd    dq 0
  clientfd  dq 0

  enable    dw 0

  server_sin_family  dw AF_INET
  server_sin_port    dw PORT
  server_sin_addr    dd INADDR_ANY
  server_sa_zero     dq 0
  server_addrlen     equ $ - server_sin_family

  client_sin_family  dw 0 
  client_sin_port    dw 0 
  client_sin_addr    dd 0 
  client_sa_zero     dq 0
  client_addrlen     dq server_addrlen

  hello db "Hello from Netwide Assembly!", LINE_FEED
  len   equ $ - hello

  request     times REQUEST_MAX_LEN db 0
  request_len dq 0

  method      times METHOD_MAX_LEN db 0
  method_len  dq 0

  route      times ROUTE_MAX_LEN db 0
  route_len  dq 0

  methods_list     db "GET POST PUT DELETE", NULL_CHAR
  methods_list_len equ $ - methods_list

  routes_list     times MAX_ROUTES_COUNT * ROUTE_MAX_LEN db 0
  routes_list_len dq 0

  test_route_1      db "/", NULL_CHAR
  test_route_1_len  equ $ - test_route_1

  test_route_2      db "/style/style.css", NULL_CHAR
  test_route_2_len  equ $ - test_route_2

  test_route_3      db "/js/index.js", NULL_CHAR
  test_route_3_len  equ $ - test_route_3

  ; responses
  response_200      db "HTTP/1.1 200 OK", CARRIAGE_RETURN, LINE_FEED 
                    db "Content-Type: text/html; charset=UTF-8", CARRIAGE_RETURN, LINE_FEED
                    db "Connection: close", CARRIAGE_RETURN, LINE_FEED
                    db CARRIAGE_RETURN, LINE_FEED
  response_200_len  equ $ - response_200

  response_405      db "HTTP/1.1 405 Method Not Allowed", CARRIAGE_RETURN, LINE_FEED
                    db "Content-Type: text/html; charset=UTF-8", CARRIAGE_RETURN, LINE_FEED
                    db "Connection: close", CARRIAGE_RETURN, LINE_FEED
                    db CARRIAGE_RETURN, LINE_FEED
  response_405_len  equ $ - response_405

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

  response_500      db "HTTP/1.1 500 Internal Server Error", CARRIAGE_RETURN, LINE_FEED
                    db "Content-Type: text/html; charset=UTF-8", CARRIAGE_RETURN, LINE_FEED
                    db "Connection: close", CARRIAGE_RETURN, LINE_FEED
                    db CARRIAGE_RETURN, LINE_FEED
  response_500_len  equ $ - response_500


section .bss

section .data
  DEFAULT_PORT equ 8080

  server_struct:
    server_sockfd             dq 0
    server_disallowed_methods dq 0
    server_routes             dq 0
    server_sockaddr           dq 0
    server_middlewares        dq 0
    server_port               dw 0
  end_server_struct:

  SERVER_STRUCT_LEN equ end_server_struct - server_struct

  ; offsets
  SERVER_OFF_SOCKFD              equ 0x0
  SERVER_OFF_DISALLOWED_METHODS  equ SERVER_OFF_SOCKFD + 0x8
  SERVER_OFF_ROUTES              equ SERVER_OFF_DISALLOWED_METHODS + 0x8
  SERVER_OFF_SOCKADDR            equ SERVER_OFF_ROUTES + 0x8
  SERVER_OFF_MIDDLEWARES         equ SERVER_OFF_SOCKADDR + 0x8
  SERVER_OFF_PORT                equ SERVER_OFF_MIDDLEWARES + 0x8

  route_struct:
    route_method    dq 0  
    route_url       dq 0
    route_callback  dq 0
    route_next      dq 0
  route_struct_end:

  ROUTE_STRUCT_LEN equ route_struct_end - route_struct

  ROUTE_OFF_METHOD    equ 0x0
  ROUTE_OFF_URL       equ ROUTE_OFF_METHOD + 0x8
  ROUTE_OFF_CALLBACK  equ ROUTE_OFF_URL + 0x8
  ROUTE_OFF_NEXT      equ ROUTE_OFF_CALLBACK + 0x8

  method_struct:
    method_str    dq 0
    method_next   dq 0
  method_struct_end:

  METHOD_STRUCT_LEN equ method_struct_end - method_struct

  METHOD_OFF_STR    equ 0x0
  METHOD_OFF_NEXT   equ METHOD_OFF_STR + 0x8

section .text
htons:
  ; rdi -> port
  ; return value: rax -> network port
  mov   rax, rdi
  xchg  al, ah ; bswap 16-bit registers
  ret

; disallows a specific method
; @param  rdi: pointer to server struct
; @param  rsi: pointer to method string
; @return rax: code
disallow_method:
  sub   rsp, 0x10

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  ; make sure the method is valid
  mov   rax, GET
  cmp   rsi, rax
  jl    .error

  mov   rax, CONNECT
  cmp   rsi, rax
  jg    .error

  ; check if the method is already in the disallowed methods ll
  mov   rsi, [rsp]
  mov   rdi, [rsi+SERVER_OFF_DISALLOWED_METHODS]

  cmp   rdi, 0
  je    .end_loop

  xor   rax, rax
.loop:
  mov   rdx, [rsp+0x8]
  cmp   rdx, [rdi+METHOD_OFF_STR]
  je    .return

  cmp   qword [rdi+METHOD_OFF_NEXT], 0
  je    .end_loop

  mov   rsi, [rdi+METHOD_OFF_NEXT]
  mov   rdi, rsi

  jmp   .loop
.end_loop:

  ; malloc method struct
  mov   rdi, METHOD_STRUCT_LEN
  call  malloc

  cmp   rax, 0
  jl    .error

  ; populate struct
  mov   rbx, [rsp+0x8]
  mov   [rax+METHOD_OFF_STR], rbx
  mov   qword [rax+METHOD_OFF_NEXT], 0

  ; add to server's linked list
  mov   rsi, [rsp]
  mov   rdi, [rsi+SERVER_OFF_DISALLOWED_METHODS]

  cmp   rdi, 0
  jne   .loop2

.init_dis_methods:
  mov   [rsi+SERVER_OFF_DISALLOWED_METHODS], rax
  mov   rax, SUCCESS_CODE
  jmp   .return

.loop2:
  cmp   qword [rdi+METHOD_OFF_NEXT], 0
  je    .end_loop2

  mov   rsi, [rdi+METHOD_OFF_NEXT]
  mov   rdi, rsi

  jmp   .loop2

.end_loop2:
  mov   [rdi+METHOD_OFF_NEXT], rax

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; create a route serving files located in a specified dir 
; @param  rdi: path
; @param  rsi: (bool) recursion
; @return rax: code
add_dir_route:
  sub   rsp, 0x40

  cmp   rdi, 0
  jle   .error

  mov   [rsp+0x8], rdi
  mov   [rsp+0x38], rsi

  ; map file_path and direntries
  mov   rdi, DIR_ENTRIES_LEN
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x28], rax

  mov   rdi, FILE_PATH_LEN
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x30], rax

  ; save dir_path into file var
  mov   rsi, [rsp+0x8]
  mov   rdi, [rsp+0x30]

  ; add slash before dir path
  mov   rax, SLASH
  stosb
  call  strcpy
  
  mov   rdi, [rsp+0x8]

  ; open directory
  mov   rsi, O_RDONLY | O_DIRECTORY
  mov   rdx, 0
  call  open_file

  cmp   rax, 0
  jl    .error

  ; get dirent struct
  mov   rdi, rax
  mov   rax, SYS_GETDENTS
  mov   rsi, [rsp+0x28]
  mov   rdx, DIR_ENTRIES_LEN
  syscall

  cmp   rax, 0
  jl    .error

  ; store number of bytes read
  mov   qword [rsp], rax

  ; save file_path_len
  mov   rdi, [rsp+0x30]
  call  strlen
  mov   qword [rsp+0x10], rax

  xor   r9, r9
  mov   rdi, [rsp+0x28]
  
.loop:
  ; d_reclen
  xor   r10, r10

  ; mov to d_reclen (offset 16)
  mov   r10w, word [rdi + 0x10]

  ; get d_type (offset d_reclen - 1)
  xor   rax, rax
  mov   al, byte [rdi + r10 - 1]

  ; check if it's a file (DT_REG = 0x8)
  cmp   rax, DT_REG
  jne   .skip_not_file

  ; save rdi pos
  mov   [rsp+0x8], rdi

  ; clear file path after dir
  mov   rdi, [rsp+0x30]
  mov   rax, qword [rsp+0x10]
  add   rdi, rax
  mov   rax, SLASH
  stosb
  call  strlen

  mov   rcx, rax
  xor   rax, rax
  rep   stosb

  mov   rdi, [rsp+0x30] 
  mov   rax, qword [rsp+0x10]
  add   rdi, rax
  inc   rdi

  mov   rsi, [rsp+0x8]

  ; go to file name (offset 18)
  add   rsi, 18

  call  strcpy

  mov   rsi, [rsp+0x30] 
  lea   rdi, [GET]
  mov   rdx, serve_dir
  call  add_route

  mov   rdi, [rsp+0x8]

  jmp   .skip_not_dir

.skip_not_file:
  cmp   rax, DT_DIR
  jne   .skip_not_dir

  ; make sure the dir is not '.'
  cmp   byte [rdi+18], DOT
  je    .skip_not_dir 

  ; save r9, r10 and rdi
  mov   qword [rsp+0x18], r9
  mov   qword [rsp+0x20], r10
  mov   [rsp+0x8], rdi

  ; clear file path after dir
  mov   rdi, [rsp+0x30]
  mov   rax, qword [rsp+0x10]
  add   rdi, rax
  mov   rax, SLASH
  stosb
  call  strlen

  mov   rcx, rax
  xor   rax, rax
  rep   stosb

  mov   rdi, [rsp+0x30] 
  mov   rax, qword [rsp+0x10]
  add   rdi, rax
  inc   rdi

  mov   rsi, [rsp+0x8]

  ; go to file name (offset 18)
  add   rsi, 18

  call  strcpy

  mov   rsi, [rsp+0x30]
  lea   rdi, [rsi]

  ; add one to remove slash at the beginning
  cmp   qword [rsp+0x38], 1
  jne   .no_recursion

  inc   rdi
  mov   rsi, 1
  call  add_dir_route

.no_recursion:
  mov   r9, qword [rsp+0x18]
  mov   r10, qword [rsp+0x20]
  mov   rdi, [rsp+0x8]

.skip_not_dir:
  add   r9, r10
  add   rdi, r10

  cmp   r9, qword [rsp]
  jge   .return
  jmp   .loop

.return:
  mov   rdi, [rsp+0x28]
  call  free

  mov   rdi, [rsp+0x30]
  call  free

  mov   rax, SUCCESS_CODE
  add   rsp, 0x40
  ret

.error:
  mov   rdi, [rsp+0x28]
  call  free

  mov   rdi, [rsp+0x30]
  call  free

  mov   rax, FAILURE_CODE
  add   rsp, 0x40
  ret

add_route:
  ; rdi -> method
  ; rsi -> route
  ; rdx -> callback
  sub   rsp, 0x20

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   qword [rsp], 0 ; method_len
  mov   qword [rsp+0x8], 0 ; route_len

  mov   [rsp+0x10], rdi 
  mov   [rsp+0x18], rsi

  ; store callback
  mov   qword [rel callback], rdx

  ; get method_length
  mov   rdi, [rsp+0x10]
  call  strlen
  mov   qword [rsp], rax

  ; get route_length
  mov   rdi, [rsp+0x18]
  call  strlen
  mov   qword [rsp+0x8], rax

  ; check if the length of the route is equal to 1
  mov   rsi, [rsp+0x18]
  cmp   qword [rsp+0x8], 1
  je    .skip

  ; if not equal, extract last char to see if it's a slash
  add   rsi, qword [rsp+0x8]
  dec   rsi
  lodsb

  cmp   al, SLASH
  jne   .skip_trim
  
  ; if last char is a '/', remove it and add null char
  mov   byte [rsi], NULL_CHAR
  ; decrease route_length
  dec   qword [rsp+0x8]
  
.skip_trim:
  ; go back to beginning of the route
  sub   rsi, qword [rsp+0x8]

.skip:
  ; go to next available space inside route_list
  xor   rdx, rdx
  mov   rax, qword [routes_count]
  mov   rbx, STRUCT_ROUTE_LEN
  mul   rbx

  lea   rdi, [routes_list] 
  add   rdi, rax
  ; move route inside the list
  mov   rcx, qword [rsp+0x8]
  rep   movsb

  ; move the method after the route
  add   rdi, URL_MAX_LEN
  sub   rdi, qword [rsp+0x8]

  ; load method inside rsi
  mov   rsi, [rsp+0x10]
  mov   rcx, qword [rsp]
  rep   movsb

  ; store callback in the right index
  xor   rdx, rdx
  lea   rdi, [callback_list]

  ; compute callback index
  mov   rax, qword [routes_count]
  mov   rbx, 8 ; size of qword
  mul   rbx
  add   rdi, rax

  mov   rax, [rel callback]
  mov   qword [rdi], rax

  ; clear callback
  mov   qword [callback], 0

  ; increase number of routes
  inc   qword [routes_count]

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret

; initialise server
; @param  rdi: port
; @return rax: pointer to server struct | error
server_init:
  sub   rsp, 0x10

  cmp   rdi, 0
  jg    .port_defined

  ; check if port is in environment variables
  lea   rdi, [PORT_ENV]
  call  getenv

  cmp   rax, 0
  jl    .default_port

  lea   rdi, [rax]
  call  stoi

  mov   rdi, rax

  cmp   rax, 0
  jg    .port_defined

.default_port:
  mov   rbx, DEFAULT_PORT
  cmp   rax, 0
  cmovl rax, rbx

  mov   rdi, rbx

.port_defined:
  mov   qword [rsp], rdi

  ; malloc server struct
  mov   rdi, SERVER_STRUCT_LEN
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; set default values
  mov   qword [rdi+SERVER_OFF_DISALLOWED_METHODS], 0
  mov   qword [rdi+SERVER_OFF_ROUTES], 0
  mov   qword [rdi+SERVER_OFF_MIDDLEWARES], 0

  ; malloc resources for server
  mov   rdi, MAX_ROUTES_COUNT
  mov   rsi, ROUTE_STRUCT_LEN
  call  calloc

  mov   rdi, [rsp+0x8]
  mov   qword [rdi+SERVER_OFF_ROUTES], rax

  ; check if port is in bounds
  cmp   qword [rsp], PORT_MIN
  jl    .error

  cmp   qword [rsp], PORT_MAX
  jg    .error

  ; add port to server struct
  mov   rax, [rsp+0x8]
  mov   rdi, qword [rsp]
  mov   dword [rax+SERVER_OFF_PORT], edi

  mov   rdi, AF_INET
  mov   rsi, SOCK_STREAM
  mov   rdx, 0
  call  create_socket

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   qword [rdi+SERVER_OFF_SOCKFD], rax

  ; set socket options
  mov   rdi, rax
  mov   rsi, SOL_SOCKET
  mov   rdx, SO_REUSEPORT
  call  set_socket_option

  cmp   rax, 0
  jl    .error

  mov   rsi, [rsp+0x8]
  add   rsi, SERVER_OFF_SOCKFD

  mov   rdi, qword [rsi]
  mov   rsi, SOL_SOCKET
  mov   rdx, SO_REUSEADDR
  call  set_socket_option

  cmp   rax, 0
  jl    .error

  mov   rdx, [rsp+0x8]
  mov   rdi, qword [rdx+SERVER_OFF_SOCKFD]
  mov   rsi, qword [rdx+SERVER_OFF_PORT]
  mov   rdx, AF_INET
  call  bind_socket

  cmp   rax, 0
  jl    .error

  ; listen socket
  mov   rsi, [rsp+0x8]
  mov   rdi, qword [rsi+SERVER_OFF_SOCKFD]
  xor   rsi, rsi
  call  listen_socket

  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x8]
  jmp   .return

.error:
  ; check if socket have been opened
  cmp   qword [rsp+0x8], 0
  jle   .skip_close_socket

  ; if yes close it
  mov   rsi, [rsp+0x8]
  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsi+SERVER_OFF_SOCKFD]
  syscall

  ; don't need to check for error, we exit anyway

.skip_close_socket:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; runs the server
; @param  rdi: pointer to server struct
; @param  rax: return value
run_server:
  sub   rsp, 0x30

  mov   [rsp], rdi
  mov   qword [rsp+0x8], 0

  call  log_server_init

.loop:
  ; set end timer
  mov   rax, SYS_CLOCK_GETTIME
  mov   rdi, CLOCK_REALTIME
  lea   rsi, [end_tv_sec]
  syscall

  ; check if we have set start timer
  cmp   qword [start_tv_sec], 0
  je    .timer_not_defined

  ; if yes, compute the difference
  mov   rax, qword [end_tv_nsec]
  sub   rax, qword [start_tv_nsec]

  mov   qword [duration], rax

  ; clear callback
  mov   qword [callback], 0

  ; log previous request
  mov   rdi, [rsp+0x28]
  call  log_request

  ; free request
  mov   rdi, [rsp+0x28]
  call  free_request

.timer_not_defined:
  mov   rcx, QUERY_MAX_LEN
  xor   rax, rax
  lea   rdi, [query_params]
  rep   stosb

  mov   rcx, HEADERS_MAX_LEN
  xor   rax, rax
  lea   rdi, [response_headers]
  rep   stosb

  mov   rcx, RESPONSE_MAX_LEN
  xor   rax, rax
  lea   rdi, [response]
  rep   stosb

  ; accept connection
  mov   rdi, [rsp]
  call  accept_connection

  cmp   rax, 0
  jl    .close

  mov   qword [rsp+0x10], rax

  ; set start timer
  mov   rax, SYS_CLOCK_GETTIME
  mov   rdi, CLOCK_REALTIME
  lea   rsi, [start_tv_sec]
  syscall

  ; malloc request string
  mov   rdi, REQUEST_MAX_LEN
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; receive client request
  mov   rax, SYS_RECVFROM
  mov   rdi, qword [rsp+0x10]
  mov   rsi, [rsp+0x18]
  mov   rdx, REQUEST_MAX_LEN
  xor   r10, r10
  xor   r9, r9
  xor   r8, r8
  syscall

  mov   qword [rsp+0x20], rax

  ; parse_request
  mov   rdi, [rsp+0x18]
  mov   rsi, qword [rsp+0x10]
  mov   rdx, qword [rsp+0x20]
  call  parse_request

  cmp   rax, 0
  jl    .error

  ; save request pointer
  mov   [rsp+0x28], rax

  mov   rdi, rax
  call  run_middlewares

  ; verify request
  mov   rdi, [rsp+0x28]
  call  verify_request
  cmp   rax, 0
  jl    .error

  cmp   qword [callback], 0
  je    .skip_callback

  mov   rdi, [rsp+0x28]
  call  [rel callback]

.skip_callback:
  mov   rax, SYS_CLOSE
  mov   rsi, [rsp+0x28] 
  mov   rdi, [rsi+REQ_OFF_CLIENT]
  syscall
  
  jmp   .loop

.error:
  mov   rax, SYS_CLOSE
  mov   rsi, [rsp+0x28] 
  mov   rdi, [rsi+REQ_OFF_CLIENT]
  syscall

  ; jump to next request, skipping logs
  jmp   .loop

.close:
  ; shutdown server
  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp]
  syscall

  add   rsp, 0x30
  ret

trim_query_parameters:
  ; rdi -> request_url
  ; rsi -> route_length
  ; return value: rax -> url length
  sub   rsp, 0x10 ; url length and query start length

  cmp   rdi, 0
  jle   .error

  mov   qword [rsp], rsi

  mov   rsi, QUESTION_MARK
  call  find_next_char
  cmp   rax, 0
  jle   .no_query

  mov   qword [rsp+0x8], rax

  add   rdi, rax
  mov   al, NULL_CHAR
  stosb

  lea   rsi, [rdi]
  lea   rdi, [query_params]
  mov   rcx, qword [rsp]
  sub   rcx, qword [rsp+0x8]
  rep   movsb

  mov   rax, qword [rsp+0x8]
  mov   qword [rsp], rax

.no_query:
  mov   rax, qword [rsp]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret


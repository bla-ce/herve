section .bss
server_struct:
  server_sockfd       resq 1  ; socket file descriptor
  server_logger       resq 1  ; pointer to the logger
  server_routes       resq 1  ; pointer to linked list of routes
  server_sockaddr     resq 1  ; pointer to sockaddr struct
  server_middlewares  resq 1  ; pointer to middlewares linked list
  server_port         resw 1  ; port
server_struct_end:

section .data
  ; server constants
  DEFAULT_MAX_CONNECTIONS equ 0x1
  DEFAULT_BACKLOG         equ 10

  PORT_MIN     equ 0x1
  DEFAULT_PORT equ 8080
  PORT_MAX     equ 0xffff

  SERVER_STRUCT_LEN equ server_struct_end - server_struct

  ; offsets
  SERVER_OFF_SOCKFD       equ 0x0
  SERVER_OFF_LOGGER       equ SERVER_OFF_SOCKFD + 0x8
  SERVER_OFF_ROUTES       equ SERVER_OFF_LOGGER + 0x8
  SERVER_OFF_SOCKADDR     equ SERVER_OFF_ROUTES + 0x8
  SERVER_OFF_MIDDLEWARES  equ SERVER_OFF_SOCKADDR + 0x8
  SERVER_OFF_PORT         equ SERVER_OFF_MIDDLEWARES + 0x8

section .text
; initial log of the available routes and port the server is listening to
; @param  rdi: pointer to server struct
log_server_init:
  sub   rsp, 0x20

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the server struct
  ; [rsp+0x8]   -> pointer to server logger
  ; [rsp+0x10]  -> port string

  mov   [rsp], rdi
  mov   qword [rsp+0x10], 0

  cmp   rdi, 0
  jle   .error

  ; get logger output
  mov   rdi, [rsp]
  call  get_server_logger
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; log the logo :)
  mov   rdi, [rsp+0x8]
  lea   rsi, [HERVE_LOGO]
  call  logln

  mov   rdi, [rsp+0x8]
  lea   rsi, [log_debug]
  call  log

  mov   rdi, [rsp+0x8]
  lea   rsi, [log_server_init_msg]
  call	log 

  mov   rsi, [rsp]
  mov   rdi, qword [rsi+SERVER_OFF_PORT]
  lea   rsi, [rsp+0x10]
  mov   rdx, STR_PORT_MAX_LENGTH
  call  to_string

  cmp   rax, 0
  jl    .return

  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call	logln

  ; load server routes
  mov   rdi, [rsp]
  call  get_server_routes
  cmp   rax, 0
  jl    .error
  je    .no_routes

  mov   [rsp+0x18], rax

  mov   rdi, [rsp+0x8]
  mov   rsi, LINE_FEED
  call  log_char

  mov   rdi, [rsp+0x8]
  lea   rsi, [log_debug]
  call  log

  mov   rdi, [rsp+0x8]
  lea   rsi, [log_routes_available_msg]
  call	logln

.loop:
  ; log debug 
  mov   rdi, [rsp+0x8]
  lea   rsi, [log_debug]
  call	log

  ; load routes_list
  mov   rdi, [rsp+0x18]

  ; log method
  ; go to method
  mov   rsi, [rdi+ROUTE_OFF_METHOD]
  mov   rdi, [rsp+0x8]
  call  log

  ; print space
  mov   rdi, [rsp+0x8]
  mov   rsi, SPACE
  call  log_char

  ; load url and log it
  mov   rdi, [rsp+0x8]
  mov   rax, [rsp+0x18]
  mov   rsi, [rax+ROUTE_OFF_URL]
  call  logln

  mov   rsi, [rsp+0x18]
  cmp   qword [rsi+ROUTE_OFF_NEXT], 0
  je    .end_loop

  add   rsi, ROUTE_OFF_NEXT
  mov   rdi, [rsi]

  mov   [rsp+0x18], rdi
  
  jmp   .loop
.end_loop:

.no_routes:
  ; log new line
  mov   rdi, [rsp+0x8]
  mov   rsi, LINE_FEED
  call	log_char

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret

; returns the socket fd associated to the server
; @param  rdi: pointer to server struct 
; @return rax: socket fd
get_server_sockfd:
  cmp   rdi, 0
  jle   .error

  mov   rax, qword [rdi+SERVER_OFF_SOCKFD]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the logger associated to the server
; @param  rdi: pointer to server struct 
; @param  rsi: pointer to the logger struct
; @return rax: return code
set_server_logger:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   qword [rdi+SERVER_OFF_LOGGER], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the logger associated to the server
; @param  rdi: pointer to server struct 
; @return rax: pointer to the logger struct
get_server_logger:
  cmp   rdi, 0
  jle   .error

  mov   rax, qword [rdi+SERVER_OFF_LOGGER]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the socket fd associated to the server
; @param  rdi: pointer to server struct 
; @param  rsi: sockfd
; @return rax: return code
set_server_sockfd:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   qword [rdi+SERVER_OFF_SOCKFD], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the port associated to the server
; @param  rdi: pointer to server struct 
; @return rax: port
get_server_port:
  cmp   rdi, 0
  jle   .error

  mov   rax, qword [rdi+SERVER_OFF_PORT]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the port the server will be listening to
; @param  rdi: pointer to server struct 
; @param  rsi: port
; @return rax: return code
set_server_port:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   dword [rdi+SERVER_OFF_PORT], esi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the first route struct associated to the server
; @param  rdi: pointer to server struct 
; @return rax: pointer to the first route struct
get_server_routes:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+SERVER_OFF_ROUTES]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the first route struct associated to the server
; @param  rdi: pointer to server struct 
; @return rax: pointer to the first middleware struct
get_server_middlewares:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+SERVER_OFF_MIDDLEWARES]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; converts a integer from host byte order to network byte order (big-endian).
; @param  rdi: integer
; @return rax: integer to network byte order
htons:
  mov   rax, rdi
  xchg  al, ah ; bswap 16-bit registers
  ret

; initialise server
; @param  rdi: port
; @return rax: pointer to server struct | error
server_init:
  sub   rsp, 0x18

  cmp   rdi, 0
  jg    .port_defined

  ; *** STACK USAGE *** ;
  ; [rsp]     -> port
  ; [rsp+0x8] -> pointer to the server struct

.default_port:
  mov   rbx, DEFAULT_PORT
  cmp   rax, 0
  cmovl rax, rbx

  mov   rdi, rbx

.port_defined:
  mov   qword [rsp], rdi
  mov   qword [rsp+0x8], 0

  ; malloc server struct
  mov   rdi, SERVER_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; set default values
  mov   qword [rax+SERVER_OFF_ROUTES], 0
  mov   qword [rax+SERVER_OFF_MIDDLEWARES], 0

  ; check if port is in bounds
  cmp   qword [rsp], PORT_MIN
  jl    .error

  cmp   qword [rsp], PORT_MAX
  jg    .error

  ; add port to server struct
  mov   rdi, [rsp+0x8]
  mov   rsi, qword [rsp]
  call  set_server_port

  cmp   rax, 0
  jl    .error

  mov   rdi, qword [rsp]
  call  socket_init
  cmp   rax, 0
  jl    .error

  mov   qword [rsp+0x10], rax

  mov   rdi, [rsp+0x8]
  mov   rsi, qword [rsp+0x10]
  call  set_server_sockfd

  cmp   rax, 0
  jl    .error  

  ; set default logger
  call  logger_default
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  set_server_logger
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x8]

  jmp   .return

.error:
  ; check if socket have been opened
  cmp   qword [rsp+0x8], 0
  jle   .skip_close_socket

  ; if yes close it
  mov   rdi, qword [rsp+0x10]
  call  close_file

  ; free server struct
  mov   rax, [rsp+0x8]
  test  rax, rax
  jz    .skip_close_socket

  mov   rdi, rax
  call  free

.skip_close_socket:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; runs the server
; @param  rdi: pointer to server struct
; @param  rax: return value
run_server:
  sub   rsp, 0x8
  sub   rsp, CLIENT_STRUCT_LEN

  ; *** STACK USAGE *** ;
  ; [rsp]     -> pointer to the server struct
  ; [rsp+0x8] -> pointer to the client struct

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

  call  log_server_init

.loop:
  ; accept connection
  mov   rdi, [rsp]
  lea   rsi, [rsp+0x8]
  call  accept_connection

  cmp   rax, 0
  jl    .close_server ; TODO: is it an error?

  mov   rdi, [rsp]
  lea   rsi, [rsp+0x8]
  call  handle_ctx
  cmp   rax, 0
  jl    .error    ; TODO: check what to do if error

  ; close connection
  lea   rdi, [rsp+0x8]
  call  get_client_fd
  cmp   rax, 0
  jl    .close_server

  mov   rdi, rax
  call  close_file

  jmp   .loop

.close_server:
  ; shutdown server
  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp]
  syscall

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  add   rsp, CLIENT_STRUCT_LEN
  ret



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
  clients times MAX_CONNECT dq 0

  ; server constants
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

  server_init_msg db "Herve started listening on port ", NULL_CHAR

  STR_PORT_MAX_LENGTH equ 6

section .text
; once the server is initialised, prints the logo, server init
; message and listening port
; @param  rdi: pointer to the server struct
; @return rax: return code
log_server_init:
  sub   rsp, 0x8
  sub   rsp, STR_PORT_MAX_LENGTH

  ; *** STACK USAGE *** ;
  ; [rsp]   -> pointer to the server struct

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

  ; print ln logo
  mov   rdi, HERVE_LOGO
  call  println
  cmp   rax, 0
  jl    .error

  ; print server init message
  mov   rdi, server_init_msg
  call  print
  cmp   rax, 0
  jl    .error

  ; get port
  mov   rdi, [rsp]
  call  get_server_port
  cmp   rax, 0
  jle   .error

  mov   rdi, rax
  lea   rsi, [rsp+0x8]
  mov   rdx, STR_PORT_MAX_LENGTH
  call  to_string
  cmp   rax, 0
  jl    .return

  lea   rdi, [rsp+0x8]
  call	println
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, STR_PORT_MAX_LENGTH
  add   rsp, 0x8
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

; sets the middlewares array associated to the server
; @param  rdi: pointer to server struct 
; @param  rsi: pointer to the middlewares array
; @return rax: return code
set_server_middlewares:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   qword [rdi+SERVER_OFF_MIDDLEWARES], rsi

  mov   rax, SUCCESS_CODE
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

  ; calloc middlewares (array of pointers)
  mov   rdi, MIDDLEWARE_MAX_COUNT
  mov   rsi, 8
  call  calloc
  cmp   rax, 0
  jl    .error

  ; set middlewares
  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  set_server_middlewares
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, qword [rsp+0x10]
  call  set_server_sockfd
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
  sub   rsp, 0x30

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the server struct
  ; [rsp+0x8]   -> pointer to main FD_SET 
  ; [rsp+0x10]  -> pointer to read FD_SET 
  ; [rsp+0x18]  -> current file descriptor
  ; [rsp+0x20]  -> number of active connections
  ; [rsp+0x28]  -> pointer to the client struct

  mov   [rsp], rdi
  mov   qword [rsp+0x8], 0
  mov   qword [rsp+0x10], 0
  mov   qword [rsp+0x18], 0
  mov   qword [rsp+0x20], 0

  cmp   rdi, 0
  jle   .error

  ; malloc main set of file descriptors
  mov   rdi, FD_SET_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; malloc read set of file descriptors
  mov   rdi, FD_SET_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; init fd set
  mov   rdi, [rsp+0x8]
  call  FD_ZERO
  cmp   rax, 0
  jl    .error

  ; get the server file descriptor
  mov   rdi, [rsp]
  call  get_server_sockfd
  cmp   rax, 0
  jl    .error

  ; set server fd
  mov   rdi, rax
  mov   rsi, [rsp+0x8]
  call  FD_SET
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  call  log_server_init
  cmp   rax, 0
  jl    .error

.loop:
  ; back up main fd set
  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp+0x8]
  mov   rcx, FD_SET_STRUCT_LEN
  rep   movsb

  mov   rdi, MAX_CONNECT
  mov   rsi, [rsp+0x10]
  call  read_select
  cmp   rax, 0
  jl    .error

  mov   qword [rsp+0x18], -1 ; it is incremented at the beginning of the inner loop

.inner_loop: 
  inc   qword [rsp+0x18]

  ; TODO: what the value of MAX CONNECT should be?
  mov   rax, MAX_CONNECT
  cmp   qword [rsp+0x18], rax
  jg    .inner_loop_end

  mov   rdi, qword [rsp+0x18]
  mov   rsi, qword [rsp+0x10]
  call  FD_ISSET
  cmp   rax, 0
  jl    .error
  je   .inner_loop

  ; get the server file descriptor
  mov   rdi, [rsp]
  call  get_server_sockfd
  cmp   rax, 0
  jl    .error

  cmp   qword [rsp+0x18], rax
  jne   .handle_ctx

  ; creating a new connection
  mov   rbx, qword [rsp+0x20]
  inc   rbx
  cmp   rbx, MAX_CONNECT
  jg    .inner_loop

  inc   qword [rsp+0x20]

  ; malloc client struct
  mov   rdi, CLIENT_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x28], rax

  ; accept connection
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x28]
  call  accept_connection
  cmp   rax, 0
  jl    .close_server ; TODO: is it an error?

  mov   rdi, [rsp+0x28]
  call  get_client_fd
  cmp   rax, 0
  jl    .error

  mov   qword [rsp+0x18], rax

  ; set the file descriptor
  mov   rdi, qword [rsp+0x18]
  mov   rsi, qword [rsp+0x8]
  call  FD_SET
  cmp   rax, 0
  jl    .error

  ; get client position in the array
  xor   rdx, rdx
  mov   rax, qword [rsp+0x20]
  dec   rax
  mov   rbx, 8
  mul   rbx

  ; add client to the array
  mov   rdi, clients
  add   rdi, rax
  mov   rsi, [rsp+0x28]
  mov   [rdi], rsi

.handle_ctx:
  ; get the client struct from the array
  mov   rdi, qword [rsp+0x18]
  mov   rsi, qword [rsp+0x20]
  call  get_client_by_fd
  cmp   rax, 0
  jle   .clear_fd

  mov   [rsp+0x28], rax

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x28]
  call  handle_ctx
  cmp   rax, 0
  jl    .clear_fd

  jmp   .inner_loop

.clear_fd:
  ; close connection
  mov   rdi, qword [rsp+0x18]
  call  close_file

  ; clear the file descriptor from the set
  mov   rdi, qword [rsp+0x18]
  mov   rsi, qword [rsp+0x8]
  call  FD_CLR
  cmp   rax, 0
  jl    .error

  mov   rdi, qword [rsp+0x18]
  mov   rsi, qword [rsp+0x20]
  call  get_client_by_fd
  cmp   rax, 0
  jle   .clear_fd

  ; free client struct
  mov   rdi, [rsp+0x28]
  call  free_client
  cmp   rax, 0
  jl    .error

  ; decrease the number of active connections
  dec   qword [rsp+0x20]

  jmp   .inner_loop

.inner_loop_end:
  jmp   .loop

.end_loop:

.close_server:
  ; shutdown server
  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp]
  syscall

.error:
  ; TODO: free allocated resources

  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x30
  ret



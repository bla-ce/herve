section .bss
server_t:
  .sockfd       resq 1  ; socket file descriptor
  .logger       resq 1  ; pointer to the logger
  .routes       resq 1  ; pointer to linked list of routes
  .404_handler  resq 1  ; pointer to the 404 handler
  .sockaddr     resq 1  ; pointer to sockaddr struct
  .groups       resq 1  ; pointer to groups linked list
  .port         resw 1  ; port
server_t_end:

section .data
  N_CLIENTS equ 10

  clients times N_CLIENTS dq 0

  ; server constants
  DEFAULT_BACKLOG         equ 10

  PORT_MIN     equ 0x1
  DEFAULT_PORT equ 1337
  PORT_MAX     equ 0xffff

  SERVER_STRUCT_LEN equ server_t_end - server_t

  ; offsets
  SERVER_OFF_SOCKFD       equ 0x0
  SERVER_OFF_LOGGER       equ SERVER_OFF_SOCKFD + 0x8
  SERVER_OFF_ROUTES       equ SERVER_OFF_LOGGER + 0x8
  SERVER_OFF_404_HANDLER  equ SERVER_OFF_ROUTES + 0x8
  SERVER_OFF_SOCKADDR     equ SERVER_OFF_404_HANDLER + 0x8
  SERVER_OFF_GROUPS       equ SERVER_OFF_SOCKADDR + 0x8
  SERVER_OFF_PORT         equ SERVER_OFF_GROUPS + 0x8

  server_init_msg db "Herve started listening on port ", NULL_CHAR
  routes_msg      db LINE_FEED, "Routes available:", NULL_CHAR

  ; healthcheck
  ok_msg      db "ok", NULL_CHAR
  mallocd_key db "mallocd (bytes)", NULL_CHAR
  mmapd_key   db "mmapd (bytes)", NULL_CHAR
  freed_key   db "freed (bytes)", NULL_CHAR

  ; messages
  message_key db "message", NULL_CHAR
  success_key db "success", NULL_CHAR
  error_key   db "error", NULL_CHAR

  memory_corrupted_msg  db "memory is corrupted, freed memory ",
                        db "is greater than allocated ",
                        db "memory", NULL_CHAR

  STR_PORT_MAX_LENGTH equ 6

section .text
; returns the root group of the server
; @param  rdi: pointer to the server
; @return rax: pointer to the root group
server_get_root_group:
  sub   rsp, 0x8

  ; *** STACK USAGE *** ;
  ; [rsp] -> pointer to the server struct

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

  ; get group offset to check if groups are init
  add   rdi, SERVER_OFF_GROUPS

  cmp   qword [rdi], 0
  je    .error

  ; load first group
  mov   rsi, [rdi]
  mov   rdi, rsi

.loop:
  ; rsi points to the address of current group
  cmp   qword [rsi+GRP_STRUCT_OFF_IS_ROOT], TRUE
  je    .loop_end

  ; go to next offset
  add   rdi, GRP_STRUCT_OFF_NEXT
  cmp   qword [rdi], 0
  je    .error

  ; rdi contains the address of the group
  mov   rsi, [rdi]
  mov   rdi, rsi

  jmp   .loop
.loop_end:

  mov   rax, rsi

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; enables logger globally
; @param  rdi: pointer to the server
; @return rax: return code
server_enable_logger:
  sub   rsp, 0x8

  ; *** STACK USAGE *** ;
  ; [rsp] -> pointer to the server struct

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

  call  get_logger_middleware
  cmp   rax, 0
  jl    .error

  ; add logger
  mov   rdi, [rsp]
  mov		rsi, NO_ARG
  mov   rdx, rax
  call  add_middleware
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; once the server is initialised, prints the logo, server init
; message and listening port
; @param  rdi: pointer to the server struct
; @return rax: return code
log_server_init:
  sub   rsp, 0x10
  sub   rsp, STR_PORT_MAX_LENGTH

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the server struct
  ; [rsp+0x8]   -> pointer to route linked list
  ; [rsp+0x10]  -> temporary buffer for listening port

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

  ; print ln logo
  mov   rdi, HERVE_LOGO
  call  print
  cmp   rax, 0
  jl    .error

  mov   rdi, routes_msg
  call  println
  cmp   rax, 0
  jl    .error

  ; print available routes
  mov   rdi, [rsp]
  call  get_server_routes
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  test  rax, rax
  jz    .loop_end

.loop:
  mov   rdi, [rsp+0x8]
  call  log_route
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x8]
  mov   rdi, [rax+ROUTE_OFF_NEXT]
  test  rdi, rdi
  jz    .loop_end

  mov   [rsp+0x8], rdi

  jmp   .loop
.loop_end:

  mov   rdi, LINE_FEED
  call  print_char
  cmp   rax, 0
  jl    .error

  ; print server init message
  mov   rdi, server_init_msg
  call  print
  cmp   rax, 0
  jl    .error

  ; get port
  mov   rdi, [rsp]
  call  server_get_port
  cmp   rax, 0
  jle   .error

  mov   rdi, rax
  lea   rsi, [rsp+0x10]
  mov   rdx, STR_PORT_MAX_LENGTH
  call  itoa
  cmp   rax, 0
  jl    .return

  ; print port
  lea   rdi, [rsp+0x10]
  call	println
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, STR_PORT_MAX_LENGTH
  add   rsp, 0x10
  ret

; returns the socket fd associated to the server
; @param  rdi: pointer to server struct
; @return rax: socket fd
server_get_sockfd:
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
server_set_logger:
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
server_get_logger:
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
server_set_sockfd:
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
server_get_port:
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
server_set_port:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  ; check if port is in bounds
  cmp   esi, PORT_MIN
  jl    .error

  cmp   esi, PORT_MAX
  jg    .error

  mov   dword [rdi+SERVER_OFF_PORT], esi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the first group struct associated to the server
; @param  rdi: pointer to server struct
; @return rax: pointer to the first group struct
server_get_groups:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+SERVER_OFF_GROUPS]
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

; converts a integer from host byte order to network byte order (big-endian).
; @param  rdi: integer
; @return rax: integer to network byte order
htons:
  xor   rax, rax
  mov   rax, rdi
  xchg  al, ah ; bswap 16-bit registers
  ret

; initialise server
; @return rax: pointer to server struct | error
server_init:
  sub   rsp, 0x8

  ; *** STACK USAGE *** ;
  ; [rsp]  -> pointer to the server struct

  mov   qword [rsp], 0

  ; malloc server struct
  mov   rdi, SERVER_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp], rax

  ; set default values
  mov   qword [rax+SERVER_OFF_ROUTES], 0
  mov   qword [rax+SERVER_OFF_404_HANDLER], 0
  mov   qword [rax+SERVER_OFF_LOGGER], 0
  mov   qword [rax+SERVER_OFF_GROUPS], 0

  ; add port to server struct
  mov   rdi, [rsp]
  mov   rsi, DEFAULT_PORT
  call  server_set_port
  cmp   rax, 0
  jl    .error

  ; create root group
  mov   rdi, [rsp]
  mov   rsi, ROOT_GRP_PREFIX
  mov   rdx, TRUE
  call  add_group
  cmp   rax, 0
  jl    .error

  ; create the healthcheck endpoint
  mov   rdi, [rsp]
  mov   rsi, GET
  mov   rdx, health_check_url
  mov   rcx, health_check
  mov		r8, NO_ARG
  mov		r9, NO_ARG
  call  add_route
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp]

  jmp   .return

.error:
  ; free server struct
  mov   rdi, [rsp]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; runs the server
; @param  rdi: pointer to server struct
; @param  rax: return value
server_run:
  sub   rsp, 0x30

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the server struct
  ; [rsp+0x8]   -> file descriptor of the server
  ; [rsp+0x10]  -> pointer to main FD_SET
  ; [rsp+0x18]  -> pointer to read FD_SET
  ; [rsp+0x20]  -> current file descriptor
  ; [rsp+0x28]  -> number of active connections
  ; [rsp+0x30]  -> pointer to the client struct

  mov   [rsp], rdi
  mov   qword [rsp+0x10], 0
  mov   qword [rsp+0x18], 0
  mov   qword [rsp+0x20], 0
  mov   qword [rsp+0x28], 0
  mov   qword [rsp+0x30], 0

  cmp   rdi, 0
  jle   .error

  ; create, bind and listen to the socket
  mov   rdi, qword [rsp]
  call  server_get_port
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  socket_init
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; malloc main set of file descriptors
  call  fd_set_init
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; malloc read set of file descriptors
  mov   rdi, FD_SET_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; set server fd
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x10]
  call  FD_SET
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  call  log_server_init
  cmp   rax, 0
  jl    .error

.loop:
  ; back up main fd set
  mov   rdi, [rsp+0x18]
  mov   rsi, [rsp+0x10]
  mov   rcx, FD_SET_STRUCT_LEN
  rep   movsb

  mov   rdi, MAX_CONNECT
  mov   rsi, [rsp+0x18]
  call  read_select
  cmp   rax, 0
  jl    .error

  mov   qword [rsp+0x20], -1 ; it is incremented at the beginning of the inner loop

.inner_loop:
  inc   qword [rsp+0x20]

  ; TODO: what the value of MAX CONNECT should be?
  mov   rax, MAX_CONNECT
  cmp   qword [rsp+0x20], rax
  jg    .inner_loop_end

  mov   rdi, qword [rsp+0x20]
  mov   rsi, qword [rsp+0x18]
  call  FD_ISSET
  cmp   rax, 0
  jl    .error
  je   .inner_loop

  ; get the file descriptor of the server
  mov   rax, [rsp+0x8]

  cmp   qword [rsp+0x20], rax
  jne   .handle_ctx

  ; creating a new connection
  mov   rbx, qword [rsp+0x28]
  inc   rbx
  cmp   rbx, MAX_CONNECT
  jg    .inner_loop

  inc   qword [rsp+0x28]

  ; accept connection
  mov   rdi, [rsp+0x8]
  call  accept_connection
  cmp   rax, 0
  jl    .close_server ; TODO: is it an error?

  mov   qword [rsp+0x20], rax

  ; set the file descriptor
  mov   rdi, qword [rsp+0x20]
  mov   rsi, qword [rsp+0x10]
  call  FD_SET
  cmp   rax, 0
  jl    .error

  ; init a new client
  mov   rdi, [rsp+0x20]
  call  client_init
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x30], rax

  ; get client position in the array
  xor   rdx, rdx
  mov   rax, qword [rsp+0x28]
  dec   rax
  mov   rbx, 8
  mul   rbx

  ; add client to the array
  mov   rdi, clients
  add   rdi, rax
  mov   rsi, [rsp+0x30]
  mov   [rdi], rsi

.handle_ctx:
  ; get the client struct from the array
  mov   rdi, qword [rsp+0x20]
  mov   rsi, qword [rsp+0x28]
  call  get_client_by_fd
  cmp   rax, 0
  jle   .clear_fd

  mov   [rsp+0x30], rax

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x30]
  call  handle_ctx
  cmp   rax, 0
  jl    .clear_fd

  jmp   .inner_loop

.clear_fd:
  mov   rdi, qword [rsp+0x20]
  mov   rsi, [rsp+0x10]
  mov   rdx, qword [rsp+0x28]
  call  client_clear_connection
  cmp   rax, 0
  jl    .error ; TODO: error?

  ; decrease the number of active connections
  dec   qword [rsp+0x28]

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

; sets the 404 handler of the server
; @param  rdi: pointer to the server struct
; @param  rsi: pointer to the handler
; @return rax: return code
set_404_handler:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   [rdi+SERVER_OFF_404_HANDLER], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns 200 OK and amount of memory mallocd, freed and mmapd
; @param  rdi: pointer to the context struct
health_check:
  sub   rsp, 0x10

  ; *** STACK USAGE *** ;
  ; [rsp] -> pointer to the context struct

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

  ; create json
  call  json_create
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rax, qword [mallocd]
  mov   rbx, qword [freed]

  cmp   rax, rbx
  jl    .failed_healthcheck

  mov   rdi, [rsp+0x8]
  mov   rsi, message_key
  mov   rdx, ok_msg
  call  json_insert_string
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, [rsp+0x8]
  mov   rsi, mallocd_key
  mov   rdx, qword [mallocd]
  call  json_insert_integer
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, [rsp+0x8]
  mov   rsi, freed_key
  mov   rdx, qword [freed]
  call  json_insert_integer
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, [rsp+0x8]
  mov   rsi, mmapd_key
  mov   rdx, qword [mmapd]
  call  json_insert_integer
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, [rsp+0x8]
  call  json_end
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, [rsp]
  mov   rsi, OK
  mov   rdx, [rsp+0x8]
  call  send_JSON
  cmp   rax, 0
  jl    .error

  ; free json
  mov   rdi, [rsp+0x8]
  call  json_free
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.failed_healthcheck:
  mov   rdi, [rsp+0x8]
  mov   rsi, error_key
  mov   rdx, memory_corrupted_msg
  call  json_insert_string
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, [rsp+0x8]
  mov   rsi, mallocd_key
  mov   rdx, qword [mallocd]
  call  json_insert_integer
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, [rsp+0x8]
  mov   rsi, freed_key
  mov   rdx, qword [freed]
  call  json_insert_integer
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, [rsp+0x8]
  mov   rsi, mmapd_key
  mov   rdx, qword [mmapd]
  call  json_insert_integer
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, [rsp+0x8]
  call  json_end
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, [rsp]
  mov   rsi, INTERNAL_SERVER_ERROR
  mov   rdx, [rsp+0x8]
  call  send_JSON
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  call  json_free
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rdi, [rsp+0x8]
  test  rdi, rdi
  jz    .no_free

  call  json_free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

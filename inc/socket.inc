section .data
  ; socket constants
  INADDR_ANY          equ 0
  SOCK_STREAM         equ 1
  SOL_SOCKET          equ 1
  SO_REUSEADDR        equ 2
  AF_INET             equ 2
  STATUS_CODE_LEN     equ 3
  MAX_ROUTES_COUNT    equ 20
  SO_REUSEPORT        equ 15
  MSG_TRUNC           equ 0x20

  enable  dw 1

  sockaddr_struct:
    sockaddr_sin_family  dw 0
    sockaddr_sin_port    dw 0
    sockaddr_sin_addr    dd 0
    sockaddr_sa_zero     dq 0
    sockaddr_addrlen     dq $ - sockaddr_sin_family
  sockaddr_struct_end:

  SOCKADDR_STRUCT_LEN equ sockaddr_struct_end - sockaddr_struct

  SOCKADDR_OFF_SIN_FAMILY  equ 0x0
  SOCKADDR_OFF_SIN_PORT    equ SOCKADDR_OFF_SIN_FAMILY + 0x2
  SOCKADDR_OFF_SIN_ADDR    equ SOCKADDR_OFF_SIN_PORT + 0x2
  SOCKADDR_OFF_SA_ZERO     equ SOCKADDR_OFF_SIN_ADDR + 0x4
  SOCKADDR_OFF_ADDRLEN     equ SOCKADDR_OFF_SA_ZERO + 0x8

section .text
; initialise the socket (create, bind and listen)
; @param  rdi: port the socket will be listening to
; @return rax: file descriptor of the socket
socket_init:
  sub   rsp, 0x10

  cmp   rdi, 0
  jle   .error

  mov   qword [rsp], rdi

  mov   rdi, AF_INET
  mov   rsi, SOCK_STREAM
  mov   rdx, 0
  call  create_socket

  cmp   rax, 0
  jl    .error

  mov   qword [rsp+0x8], rax

  ; set socket options
  mov   rdi, qword [rsp+0x8]
  mov   rsi, SOL_SOCKET
  mov   rdx, SO_REUSEPORT
  call  set_socket_option

  cmp   rax, 0
  jl    .error

  mov   rdi, qword [rsp+0x8]
  mov   rsi, SOL_SOCKET
  mov   rdx, SO_REUSEADDR
  call  set_socket_option

  cmp   rax, 0
  jl    .error

  mov   rsi, qword [rsp]
  mov   rdi, qword [rsp+0x8]
  mov   rdx, AF_INET
  call  bind_socket

  cmp   rax, 0
  jl    .error

  ; listen socket
  mov   rdi, qword [rsp+0x8]
  mov   rsi, DEFAULT_BACKLOG
  call  listen_socket

  cmp   rax, 0
  jl    .error

  mov   rax, qword [rsp+0x8]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

  
; creates socket
; @param  rdi: family
; @param  rsi: type
; @param  rdx: protocol
; @return rax: socket fd
create_socket:
  mov   rax, SYS_SOCKET
  syscall

  ret

; sets socket options
; @param  rdi: socket fd
; @param  rsi: level
; @param  rdx: optname
; @return rax: return value
set_socket_option:
  mov   rax, SYS_SETSOCKOPT
  mov   r10, enable
  mov   r8, 4
  syscall

  ret

; binds socket
; @param  rdi: socket fd
; @param  rsi: port
; @param  rdx: family
; @param  rax: return value
bind_socket:
  sub   rsp, 0x20

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   qword [rsp+0x18], 0

  ; malloc sockaddr struct
  mov   rdi, SOCKADDR_STRUCT_LEN
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; convert port to network port
  xor   rax, rax
  mov   rdi, qword [rsp+0x8]
  call  htons

  mov   rdi, [rsp+0x18]
  mov   word [rdi+SOCKADDR_OFF_SIN_PORT], ax

  mov   rax, qword [rsp+0x10]
  mov   word [rdi+SOCKADDR_OFF_SIN_FAMILY], ax

  mov   dword [rdi+SOCKADDR_OFF_SIN_ADDR], INADDR_ANY
  mov   qword [rdi+SOCKADDR_OFF_SA_ZERO], 0

  mov   rax, SYS_BIND
  mov   rsi, rdi 
  mov   rdi, qword [rsp]
  mov   rdx, qword [sockaddr_addrlen]
  syscall

  cmp   rax, 0
  jl    .error

  ; free socket addr
  mov   rdi, [rsp+0x18]
  call  free

  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, [rsp+0x18]
  test  rax, rax
  jz    .no_free
  
  mov   rdi, rax
  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret

; listens to the socket
; @param  rdi: socket fd
; @param  rsi: backlog
; @return rax: return value
listen_socket:
  mov   rax, SYS_LISTEN
  syscall

  ret

; accepts a connection and returns the client fd
; @param  rdi: pointer to server
; @return rax: client fd
accept_connection:
  sub   rsp, 0x18

  mov   [rsp], rdi
  mov   qword [rsp+0x8], 0

  mov   rdi, SOCKADDR_STRUCT_LEN 
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   word [rax+SOCKADDR_OFF_SIN_FAMILY], AF_INET
  mov   word [rax+SOCKADDR_OFF_SIN_PORT], 0
  mov   dword [rax+SOCKADDR_OFF_SIN_ADDR], 0
  mov   qword [rax+SOCKADDR_OFF_SA_ZERO], 0

  mov   rbx, qword [sockaddr_addrlen]
  mov   qword [rax+SOCKADDR_OFF_ADDRLEN], rbx

  ; get server from context
  mov   rdi, [rsp]
  call  get_server_sockfd 

  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rax, SYS_ACCEPT
  mov   rsi, [rsp+0x8]
  lea   rdx, [sockaddr_addrlen]
  syscall

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; free client address -> might need it after
  mov   rdi, [rsp+0x8]
  call  free

  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x10]

  jmp   .return

.error:
  cmp   qword [rsp+0x8], 0
  je    .no_free

  mov   rdi, [rsp+0x8]
  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret


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

  jmp   .return

.error:
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

; accepts a connection and store client fd into client struct
; @param  rdi: pointer to context
; @return rax: return code
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
  call  get_ctx_server

  cmp   rax, 0
  jl    .error

  mov   rdi, rax
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

  ; get client from the context
  mov   rdi, [rsp]
  call  get_ctx_client

  cmp   rax, 0
  jl    .error

  ; save client fd into the client struct
  mov   rdi, rax
  mov   rsi, [rsp+0x10]
  call  set_client_fd

  mov   rdi, [rsp+0x8]
  call  free

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  cmp   qword [rsp+0x8], 0
  je    .skip_free

  mov   rdi, [rsp+0x8]
  call  free

.skip_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret


global _start

STDOUT  equ 2

; socket constants
INADDR_ANY    equ 0
SOCK_STREAM   equ 1
SOL_SOCKET    equ 1
BACKLOG       equ 1
SO_REUSEADDR  equ 2
SO_REUSEPORT  equ 15
AF_INET       equ 2
PORT          equ 14597 

; syscall values
SYS_WRITE       equ 1
SYS_CLOSE       equ 3
SYS_SOCKET      equ 41
SYS_ACCEPT      equ 43
SYS_SENDTO      equ 44
SYS_BIND        equ 49
SYS_LISTEN      equ 50
SYS_SETSOCKOPT  equ 54
SYS_EXIT        equ 60

; ascii
LINE_FEED equ 10

; exit code
SUCCESS_CODE equ 0
FAILURE_CODE equ -1

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
  jl error

  mov   rax, SYS_SETSOCKOPT
  mov   rdi, [sockfd]
  mov   rsi, SOL_SOCKET
  mov   rdx, SO_REUSEADDR
  mov   r10, enable
  mov   r8, 4 
  syscall

  cmp   rax, 0
  jl error

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

  ; send back message
  mov   rax, SYS_SENDTO
  mov   rdi, [clientfd]
  lea   rsi, [hello]
  mov   rdx, len
  xor   r10, r10
  xor   r9, r9
  xor   r8, r8
  syscall

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

error:
  ; close sockets
  mov   rax, SYS_CLOSE
  mov   rdi, [sockfd] 
  syscall

  mov   rax, SYS_CLOSE
  mov   rdi, [clientfd] 
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


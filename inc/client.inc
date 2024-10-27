section .bss
  clientfd resq 1
section .data
  CLIENT_PORT equ 80
section .text

; performs get request
; @param  rdi: url
; @return rax: response
get:
  call  client_init

  cmp   rax, 0 
  jl    .error

  

.error:
  mov   rax, FAILURE_CODE

.return:

  ret


; initialize client to do requests
; @return rax: clientfd
client_init:
  ; create socket
  mov   rax, SYS_SOCKET
  mov   edi, AF_INET
  mov   rsi, SOCK_STREAM
  mov   rdx, 0
  syscall

  cmp   rax, 0
  jl    .error

  mov   qword [clientfd], rax

  mov   word [client_sin_port], CLIENT_PORT
  
  ; connect socket
  mov   rax, SYS_CONNECT
  mov   rdi, qword [clientfd]
  lea   rsi, [client_sin_family]
  mov   rdx, client_addrlen
  syscall

  mov   rax, qword [clientfd]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return
  ret


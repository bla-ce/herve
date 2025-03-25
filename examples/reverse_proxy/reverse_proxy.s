global _start

%include "herve.inc"

section .text

_start:
  sub   rsp, 0x10

  mov   rdi, 1323
  call  server_init
  cmp   rax, 0
  jl    error

  mov   [rsp], rdi

  mov   rdi, PROXY_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    error

  mov   rdi, qword [proxy_port]
  mov   word [rax+PROXY_OFF_PORT], di

  lea   rdi, [proxy_ip]
  mov   [rax+PROXY_OFF_IP], rdi

  mov   rdi, [rsp]
  mov   rsi, proxy_middleware
  mov   rdx, rax 
  call  add_middleware
  cmp   rax, 0
  jl    error

  mov   rdi, [rsp]
  call  run_server
  cmp   rax, 0
  jl    error

  call  exit

exit:
  mov   rax, SYS_EXIT
  mov   rdi, SUCCESS_CODE
  syscall

error:
  mov   rax, SYS_EXIT
  mov   rdi, FAILURE_CODE
  syscall

section .data
  proxy_port  dw 1337
  proxy_ip    db "192.168.122.129", NULL_CHAR


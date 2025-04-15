global _start

%include "herve.inc"

section .text

_start:
  sub   rsp, 0x18

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the server struct
  ; [rsp+0x8]   -> array of proxy struct (length of 2)

  mov   rdi, 4000
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

  mov   [rsp+0x8], rax
  mov   qword [proxy_array], rax

  mov   rdi, PROXY_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    error

  mov   rdi, qword [proxy2_port]
  mov   word [rax+PROXY_OFF_PORT], di

  lea   rdi, [proxy_ip]
  mov   [rax+PROXY_OFF_IP], rdi

  mov   [rsp+0x10], rax
  mov   qword [proxy_array+8], rax

  mov   rdi, [rsp]
  mov   rsi, proxy_middleware
  mov   rdx, proxy_array
  mov   rcx, ROUND_ROBIN_IP
  mov   r8, 2
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
  proxy2_port dw 1338
  proxy_ip    db "192.168.122.129", NULL_CHAR

  proxy_array times 2 dq 0


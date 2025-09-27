global _start

%include "herve.inc"
%include "proxy.inc"
%include "os.inc"

section .text

_start:
  sub   rsp, 0x10

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the server struct
  ; [rsp+0x10]  -> pointer to the middleware struct

  mov   rdi, 4000
  call  server_init
  cmp   rax, 0
  jl    .error

  mov   [rsp], rax

  mov   rdi, PROXY_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   rdi, qword [proxy_port]
  mov   word [rax+PROXY_OFF_PORT], di
  mov   dword [rax+PROXY_OFF_WEIGHT], 2

  lea   rdi, [proxy_ip]
  mov   [rax+PROXY_OFF_IP], rdi

  mov   qword [proxy_array], rax

  mov   rdi, PROXY_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   rdi, qword [proxy2_port]
  mov   word [rax+PROXY_OFF_PORT], di
  mov   dword [rax+PROXY_OFF_WEIGHT], 3

  lea   rdi, [proxy_ip]
  mov   [rax+PROXY_OFF_IP], rdi

  mov   qword [proxy_array+8], rax

  mov   rdi, proxy_middleware
  mov   rsi, proxy_array
  mov   rdx, WEIGHTED_ROUND_ROBIN_IP
  mov   rcx, PROXY_COUNT
  mov   r9, FALSE
  call  create_middleware
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  xor   rsi, rsi
  mov   rdx, rax
  call  add_middleware
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  call  run_server
  cmp   rax, 0
  jl    .error

  mov   rdi, SUCCESS_CODE
  call  exit

.error:
  mov   rdi, FAILURE_CODE
  call  exit

section .data
  proxy_port  dw 1337
  proxy2_port dw 1338
  proxy_ip    db "127.0.0.1", NULL_CHAR

  proxy_array times 2 dq 0

  PROXY_COUNT equ 2


global _start

%include "herve.inc"
%include "os.inc"

section .text

health:
  mov   rsi, OK
  lea   rdx, [ok_msg]   
  call  send_string
  ret

_start:
  sub   rsp, 0x8

  mov   rdi, 1338
  call  server_init
  cmp   rax, 0
  jl    .error

  mov   [rsp], rax

  ; get default logger
  call  logan_init
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; malloc middleware for logging
  mov   rdi, MIDDLEWARE_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x10]

  mov   qword [rax+MIDDLEWARE_OFF_ADDR], log_ctx
  mov   qword [rax+MIDDLEWARE_OFF_ARG1], rdi
  mov   qword [rax+MIDDLEWARE_OFF_ARG2], 0xFF
  mov   qword [rax+MIDDLEWARE_OFF_ARG3], 0
  mov   qword [rax+MIDDLEWARE_OFF_POST_REQ], TRUE

  mov   rdi, [rsp]
  xor   rsi, rsi
  mov   rdx, rax
  call  add_middleware
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, GET
  lea   rdx, [health_url]
  mov   rcx, health
  xor   r8, r8
  call  add_route
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  call  run_server

  mov   rdi, SUCCESS_CODE
  call  exit

.error:
  mov   rdi, FAILURE_CODE
  call  exit

section .data
  health_url  db "/health", NULL_CHAR
  ok_msg      db "ok", NULL_CHAR
  

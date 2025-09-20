global _start

%include "herve.inc"
%include "os.inc"

section .text
_start:
  sub   rsp, 0x18

  mov   rdi, 1337
  call  server_init
  cmp   rax, 0
  jl    .error

  mov   [rsp], rax

  ; get default logger
  call  logan_init
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; malloc middleware for logging
  mov   rdi, MIDDLEWARE_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]

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
  mov   rsi, dist_path
  mov   rdx, TRUE
  call  add_dir_route
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
  dist_path db "frontend/dist", NULL_CHAR


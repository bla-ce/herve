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

  mov   rdi, log_ctx
  mov   rsi, [rsp+0x8]
  mov   rdx, 0xFF
  mov   rcx, 0
  mov   r9, TRUE
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


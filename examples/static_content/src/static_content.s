global _start

%include "herve.inc"

section .text
_start:
  call  server_init
  cmp   rax, 0
  jl    .error

  mov   [server], rax

  mov   rdi, [server]
  call  server_enable_logger
  cmp   rax, 0
  jl    .error

  ; add echo route
  mov   rdi, [server]
  mov   rsi, public_path
  call  add_dir_route
  cmp   rax, 0
  jl    .error

  mov   rdi, [server]
  call  server_run

  mov   rdi, SUCCESS_CODE
  call  exit

.error:
  mov   rdi, FAILURE_CODE
  call  exit

section .data
  server dq 0

  public_path db "public/", NULL_CHAR


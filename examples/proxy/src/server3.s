global _start

%include "herve.inc"

section .text
_start:
  call  server_init
  cmp   rax, 0
  jl    .error

  mov   [server], rax

  mov   rdi, [server]
  mov   rsi, 4002
  call  server_set_port
  cmp   rax, 0
  jl    .error

  mov   rdi, [server]
  call  server_enable_logger
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



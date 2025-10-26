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

  mov   rdi, person_model_name
  call  model_create
  cmp   rax, 0
  jl    .error

  mov   [person_model], rax

  mov   rdi, [server]
  mov   rsi, [person_model]
  call  add_model_routes
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

  person_model_name db "person", NULL_CHAR

  person_model dq 0


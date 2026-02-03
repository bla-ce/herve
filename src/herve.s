%include "herve.inc"

global _start

section .data

herve dq 0

PORT equ 5000

section .text

_start:
  ; init server
  mov   rdi, PORT
  call  server_init
  cmp   rax, 0
  jl    .error

  mov   [herve], rax

  mov   rdi, [herve]
  call  server_enable_logger
  cmp   rax, 0
  jl    .error

  mov   rdi, [herve]
  call  server_run
  cmp   rax, 0
  jl    .error

  jmp  .exit

.error:
  mov   rdi, FAILURE_CODE
  call  exit

.exit:
  mov   rdi, SUCCESS_CODE
  call  exit

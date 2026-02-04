%include "herve.inc"
%include "service.inc"

global _start

section .data

herve dq 0

PORT equ 5000

; endpoints
register_url    db "/service/register", NULL_CHAR
unregister_url  db "/service/unregister", NULL_CHAR

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

  ; create service endpoints
  mov   rdi, [herve]
  mov   rsi, POST
  mov   rdx, register_url
  mov   rcx, register_service
  mov   r8, NO_ARG
  call  add_route
  cmp   rax, 0
  jl    .error

  mov   rdi, [herve]
  mov   rsi, POST
  mov   rdx, unregister_url
  mov   rcx, unregister_service
  mov   r8, NO_ARG
  call  add_route
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

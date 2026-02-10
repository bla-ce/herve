%include "herve.inc"
%include "service.inc"

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

  ; create service endpoints
  mov   rdi, [herve]
  mov   rsi, POST
  mov   rdx, service_endpoint.register
  mov   rcx, service_register
  mov   r8, NO_ARG
  call  add_route
  cmp   rax, 0
  jl    .error

  mov   rdi, [herve]
  mov   rsi, POST
  mov   rdx, service_endpoint.unregister
  mov   rcx, service_unregister
  mov   r8, NO_ARG
  call  add_route
  cmp   rax, 0
  jl    .error

  mov   rdi, [herve]
  mov   rsi, GET
  mov   rdx, service_endpoint.root
  mov   rcx, service_list
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

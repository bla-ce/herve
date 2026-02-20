%include "herve.inc"
%include "service.inc"

global _start

section .data

herve dq 0

PORT equ 5000

env_ht dq 0

section .text

_start:
  ; read env var
  mov   rdi, NO_ARG
  call  env_read

  mov   [env_ht], rax

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
  call  route_add
  cmp   rax, 0
  jl    .error

  mov   rdi, [herve]
  mov   rsi, POST
  mov   rdx, service_endpoint.unregister
  mov   rcx, service_unregister
  mov   r8, NO_ARG
  call  route_add
  cmp   rax, 0
  jl    .error

  mov   rdi, [herve]
  mov   rsi, POST
  mov   rdx, service_endpoint.start
  mov   rcx, service_start
  mov   r8, NO_ARG
  call  route_add
  cmp   rax, 0
  jl    .error

  mov   rdi, [herve]
  mov   rsi, POST
  mov   rdx, service_endpoint.stop
  mov   rcx, service_stop
  mov   r8, NO_ARG
  call  route_add
  cmp   rax, 0
  jl    .error

  mov   rdi, [herve]
  mov   rsi, GET
  mov   rdx, service_endpoint.service_root
  mov   rcx, service_list
  mov   r8, NO_ARG
  call  route_add
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

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

  mov   rdi, 1337
  call  server_init
  cmp   rax, 0
  jl    error

  mov   [rsp], rax

  mov   rdi, rax
  mov   rsi, GET
  lea   rdx, [health_url]
  mov   rcx, health
  call  add_route
  cmp   rax, 0
  jl    error

  mov   rdi, [rsp]
  call  run_server

  mov   rdi, SUCCESS_CODE
  call  exit

error:
  mov   rdi, FAILURE_CODE
  call  exit

section .data
  health_url  db "/health", NULL_CHAR
  ok_msg      db "ok", NULL_CHAR
  

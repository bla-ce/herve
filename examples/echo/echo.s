global _start

%include "herve.inc"

section .text

echo:
  sub   rsp, 0x8

  mov   [rsp], rdi

  call  get_ctx_request
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  get_request_body
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, OK
  mov   rdx, rax
  call  send_string

  jmp   .return

.error:
  mov   rdi, [rsp] 
  mov   rsi, INTERNAL_SERVER_ERROR
  call  send_no_content

  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

_start:
  sub   rsp, 0x8

  mov   rdi, 1337
  call  server_init
  cmp   rax, 0
  jl    error

  mov   [rsp], rax 

  mov   rdi, rax
  lea   rsi, [POST]
  lea   rdx, [wildcard_url]
  mov   rcx, echo
  call  add_route
  cmp   rax, 0
  jl    error

  mov   rdi, [rsp]
  call  run_server
  cmp   rax, 0
  jl    error

  mov   rax, SYS_EXIT
  mov   rdi, SUCCESS_CODE
  syscall

error:
  mov   rax, SYS_EXIT
  mov   rdi, FAILURE_CODE
  syscall

section .data
  wildcard_url  db "*", NULL_CHAR
  

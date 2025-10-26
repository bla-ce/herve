global _start

%include "herve.inc"

section .text
; returns the content of the body
; @param  rdi: pointer to the context struct
; @return rax: return code
echo_handler:
  sub   rsp, 0x8

  ; *** STACK USAGE *** ;
  ; [rsp] -> pointer to the context struct

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

  ; get request
  mov   rdi, [rsp]
  call  get_ctx_request
  cmp   rax, 0
  jl    .error

  ; get request body
  mov   rdi, rax
  call  get_request_body
  cmp   rax, 0
  jl    .error

  ; send body
  mov   rdi, [rsp]
  mov   rsi, OK
  mov   rdx, rax
  call  send_string
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

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
  mov   rsi, POST
  mov   rdx, echo_url
  mov   rcx, echo_handler
  xor   r8, r8
  call  add_route
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

  echo_url db "/", NULL_CHAR


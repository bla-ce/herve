global _start

%include "herve.inc"
%include "os.inc"

section .text

test_string:
  mov   rsi, OK  
  lea   rdx, [ok_msg]
  call  send_string
  cmp   rax, 0
  jl    .error

  jmp   .return

.error: 
  mov   rax, FAILURE_CODE

.return:
  ret

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

  mov   rdi, [rsp]
  lea   rsi, [POST]
  lea   rdx, [wildcard_url]
  mov   rcx, echo
  xor   r8, r8
  call  add_route
  cmp   rax, 0
  jl    error

  ; add dynamic route
  mov   rdi, [rsp]
  lea   rsi, [GET]
  lea   rdx, [dynamic_url]
  mov   rcx, test_string
  xor   r8, r8
  call  add_route
  cmp   rax, 0
  jl    error

  mov   rdi, [rsp]
  call  run_server
  cmp   rax, 0
  jl    error

  mov   rdi, SUCCESS_CODE
  call  exit

error:
  mov   rdi, FAILURE_CODE
  call  exit
  
section .data
  wildcard_url  db "*", NULL_CHAR
  dynamic_url   db "/api/events/<id", NULL_CHAR
  
  ok_msg db "ok", NULL_CHAR

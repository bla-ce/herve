global  _start

%include "bytasm.inc"

section .text
test_no_content:
  call  send_no_content
  ret

test_string:
  mov   rsi, OK  
  lea   rdx, [ok_msg]
  call  send_string
  ret

test_static:
  lea   rsi, [index_path]
  call  send_static_file
  ret

_start:
  sub   rsp, 0x8

  mov   rdi, 1337
  call  server_init
  cmp   rax, 0
  jl    .error

  mov   [rsp], rax

  mov   rdi, rax
  call  get_server_sockfd

  cmp   rax, 0
  jl    .error

  mov   qword [sockfd], rax

  ; add no content route
  mov   rdi, [rsp]
  lea   rsi, [GET]
  lea   rdx, [root_url]
  mov   rcx, test_no_content
  call  add_route

  cmp   rax, 0
  jl    .error

  ; add health route
  mov   rdi, [rsp]
  lea   rsi, [GET]
  lea   rdx, [health_url]
  mov   rcx, test_string
  call  add_route

  ; add health route
  mov   rdi, [rsp]
  lea   rsi, [GET]
  lea   rdx, [index_url]
  mov   rcx, test_static
  call  add_route

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  call  run_server

  add   rsp, 0x8

  mov   rax, SYS_EXIT
  mov   rdi, SUCCESS_CODE
  syscall

.error:
  add   rsp, 0x8

  mov   rax, SYS_EXIT
  mov   rdi, FAILURE_CODE
  syscall

section .bss

section .data
  sockfd  dq 0

  root_url    db "/", NULL_CHAR
  health_url  db "/health", NULL_CHAR
  index_url  db "/index", NULL_CHAR

  index_path db "examples/views/index.html", NULL_CHAR

  ok_msg db "ok", NULL_CHAR



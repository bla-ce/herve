global  _start

%include "bytasm.inc"

section .text
middleware:
  lea   rdi, [middleware_msg]  
  mov   rsi, 0
  call  println

  ret

test_no_content:
  sub   rsp, 0x8

  mov   [rsp], rdi

  mov   rdi, [rsp]
  call  get_ctx_request

  cmp   rax, 0
  jl    .error

  ; get query
  mov   rdi, rax
  lea   rsi, [name_query]
  call  get_query_param

  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, 0
  call  println

  mov   rdi, [rsp]
  call  send_no_content

  jmp   .return

.error:
  mov   rdi, [rsp]
  mov   rsi, BAD_REQUEST
  lea   rdx, [error_no_query]
  call  send_string

  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

test_string:
  sub   rsp, 0x8

  mov   [rsp], rdi

  ; test print request headers
  mov   rdi, [rsp]
  call  get_ctx_request

  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  get_request_headers

  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, 0
  call  println

  mov   rdi, [rsp]
  mov   rsi, OK  
  lea   rdx, [ok_msg]
  call  send_string

  jmp   .return

.error: 
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
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

  mov   rdi, [rsp]
  lea   rsi, [GET]
  lea   rdx, [index_url]
  mov   rcx, test_static
  call  add_route

  cmp   rax, 0
  jl    .error

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

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  lea   rsi, [dir_path]
  mov   rdx, 1
  call  add_dir_route

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, middleware
  call  add_middleware

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, middleware
  call  add_middleware

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
  index_url   db "/index", NULL_CHAR

  index_path    db "examples/views/index.html", NULL_CHAR
  dir_path      db "examples/views", NULL_CHAR

  name_query db "name", NULL_CHAR

  ok_msg          db "ok", NULL_CHAR
  middleware_msg  db "Hello, World!", NULL_CHAR

  error_no_query  db "failed to get query parameter", NULL_CHAR


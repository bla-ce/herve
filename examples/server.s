global  _start

%include "bytasm.inc"

section .text
print_hello:
  lea   rdi, [hello]
  mov   rsi, 0
  call  println

  ret

cookie_func:
  lea   rdi, [ck_test_name] 
  call  is_cookie_name_valid
  ret 

header:
  sub   rsp, 0x8

  mov   [rsp], rdi

  call  get_request_headers

  mov   rdi, rax
  mov   rsi, 0
  call  println

  lea   rdi, [name_param]
  call  get_param

  ; TODO: if failed?

  lea   rdi, [rax]
  mov   rsi, 0
  call  println

  lea   rdi, [STR_OK]
  mov   rsi, 0
  mov   rdx, 0
  mov   rcx, [rsp]
  call  send_response

  add   rsp, 0x8

  ret

; @param rdi: request
health:
  sub   rsp, 0x8
  mov   [rsp], rdi

  call  get_headers
  mov   [headers], rax

  mov   rdi, [headers]
  lea   rsi, [header_key]
  lea   rdx, [header_value]
  call  set_header

  mov   rdi, [headers]
  lea   rsi, [header_key]
  lea   rdx, [header2_value]
  call  set_header

  mov   rdi, [headers]
  lea   rsi, [header_key]
  call  get_header_value
  cmp   rax, 0
  jl    error

  mov   rdi, rax
  mov   rsi, 0
  call  println

  lea   rdi, [ok]
  mov   rsi, [rsp]
  call  serve_string

  add   rsp, 0x8
  ret

post:
  sub   rsp, 0x8
  
  mov   [rsp], rdi

  call  get_body
  lea   rdi, [rax]
  mov   rsi, 0
  call  println

  lea   rdi, [STR_OK]
  mov   rsi, 0
  mov   rdx, 0
  mov   rcx, [rsp]
  call  send_response

  add   rsp, 0x8
  ret

index:
  sub   rsp, 0x8

  mov   rdi, [rsp]

  lea   rdi, [index_path]
  mov   rsi, [rsp]
  call  serve_static_file

  add   rsp, 0x8

  ret

send_200:
  sub   rsp, 0x8
  mov   [rsp], rdi

  lea   rdi, [STR_OK]
  mov   rsi, 0
  mov   rdx, 0
  mov   rcx, [rsp]
  call  send_response

  add   rsp, 0x8
  ret

_start:
  mov   rdi, 1337
  call  server_init
  cmp   rax, 0
  jl    error

  mov   qword [sockfd], rax

  mov   rdi, print_hello
  mov   rsi, qword [sockfd]
  call  add_middleware

  mov   rdi, print_hello
  mov   rsi, qword [sockfd]
  call  add_middleware

  mov   rdi, print_hello
  mov   rsi, qword [sockfd]
  call  add_middleware

  lea   rdi, [CONNECT]
  call  disallow_method
  cmp   rax, 0
  jl    error

  lea   rdi, [DELETE] 
  call  disallow_method

  lea   rdi, [POST]
  lea   rsi, [post_route]
  mov   rdx, post
  call  add_route 

  lea   rdi, [GET]
  lea   rsi, [v1_route]
  mov   rdx, send_200
  call  add_route 

  lea   rdi, [GET]
  lea   rsi, [header_route]
  mov   rdx, header
  call  add_route 

  lea   rdi, [GET]
  lea   rsi, [api_route]
  mov   rdx, send_200
  call  add_route 

  lea   rdi, [GET]
  lea   rsi, [health_route]
  mov   rdx, health
  call  add_route 

  lea   rdi, [GET]
  lea   rsi, [index_route]
  mov   rdx, index
  call  add_route

  lea   rdi, [GET]
  lea   rsi, [cookie_route]
  mov   rdx, cookie_func
  call  add_route 

  lea   rdi, [views_dir]
  mov   rsi, 1
  call  add_dir_route
 
  mov   rdi, qword [sockfd]
  call  run_server

  mov   rax, SYS_EXIT
  mov   rdi, SUCCESS_CODE
  syscall

error:
  mov   rax, SYS_EXIT
  mov   rdi, FAILURE_CODE
  syscall

section .bss
  headers resq 1

section .data
  sockfd  dq 0

  root_route    db "/", NULL_CHAR
  health_route  db "/health", NULL_CHAR
  post_route    db "/post", NULL_CHAR
  index_route   db "/index", NULL_CHAR
  header_route  db "/header", NULL_CHAR
  cookie_route  db "/cookie", NULL_CHAR

  api_route db "/api", NULL_CHAR
  v1_route  db "/api/v1", NULL_CHAR

  views_dir   db "examples/views", NULL_CHAR
  index_path  db "examples/views/index.html", NULL_CHAR

  ok  db "ok", NULL_CHAR

  name_param db "name", NULL_CHAR

  header_key    db "set-cookie", NULL_CHAR
  header_value  db "value", NULL_CHAR
  header2_value db "value2", NULL_CHAR

  ck_test_name db "cookie=value", NULL_CHAR

  hello db "Hello, World!", NULL_CHAR

  log_file db "app.log", NULL_CHAR


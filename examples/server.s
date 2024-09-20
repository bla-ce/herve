global  _start

%include "bytasm.inc"

section .text
health:
  ; rdi -> request
  lea   rdi, [ok]
  call  serve_string

  ret

index:
  ; rdi -> request
  lea   rdi, [index_path]
  lea   rsi, [CONTENT_HTML]
  call  serve_static_file

  ret

css:
  ; rdi -> request
  lea   rdi, [css_path]
  lea   rsi, [CONTENT_CSS]
  call  serve_static_file

  ret

js:
  ; rdi -> request
  lea   rdi, [js_path]
  lea   rsi, [CONTENT_JS]
  call  serve_static_file

  ret

post:
  ; rdi -> request
  call  get_body
  lea   rdi, [rax]
  mov   rsi, 0
  call  println

  lea   rdi, [STR_OK]
  mov   rsi, 0
  mov   rdx, 0
  call  send_response

  ret

send_200:
  lea   rdi, [STR_OK]
  mov   rsi, 0
  mov   rdx, 0
  call  send_response

  ret

_start:

  mov   rdi, 1337
  call  server_init
  cmp   rax, 0
  jl    error

  mov   qword [sockfd], rax

  mov   rdi, 1
  call  set_max_connections
  cmp   rax, 0
  jl    error

  lea   rdi, [CONNECT]
  call  disallow_method
  cmp   rax, 0
  jl    error

  lea   rdi, [DELETE] 
  call  disallow_method

  lea   rdi, [GET]
  lea   rsi, [index_route]
  mov   rdx, index
  call  add_route 

  lea   rdi, [GET]
  lea   rsi, [style_route]
  mov   rdx, css
  call  add_route 

  lea   rdi, [GET]
  lea   rsi, [js_route]
  mov   rdx, js 
  call  add_route 

  lea   rdi, [POST]
  lea   rsi, [post_route]
  mov   rdx, post
  call  add_route 

  lea   rdi, [GET]
  lea   rsi, [v1_route]
  mov   rdx, send_200
  call  add_route 

  lea   rdi, [GET]
  lea   rsi, [api_route]
  mov   rdx, send_200
  call  add_route 

  lea   rdi, [GET]
  lea   rsi, [health_route]
  mov   rdx, health
  call  add_route 

  mov   rdi, qword [sockfd]
  call  run_server

  mov   rax, SYS_EXIT
  mov   rdi, SUCCESS_CODE
  syscall

error:
  mov   rdi, [errno]
  mov   rsi, 0
  call  perror

  mov   rax, SYS_EXIT
  mov   rdi, FAILURE_CODE
  syscall

section .data
  sockfd  dq 0

  root_route    db "/", NULL_CHAR
  index_route   db "/index", NULL_CHAR
  health_route  db "/health", NULL_CHAR
  post_route    db "/post", NULL_CHAR
  style_route   db "/style.css", NULL_CHAR
  js_route      db "/index.js", NULL_CHAR

  api_route db "/api", NULL_CHAR
  v1_route  db "/api/v1", NULL_CHAR

  ok  db "ok", NULL_CHAR

  index_path  db "examples/views/index.html", NULL_CHAR
  css_path    db "examples/views/style.css", NULL_CHAR
  js_path     db "examples/views/index.js", NULL_CHAR


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
  call  serve_html

  ret

post:
  ; rdi -> request
  lea   rdi, [post_path]
  call  serve_html

  ret

_start:
  mov   rdi, 1337
  call  server_init
  cmp   rax, 0
  jl    error

  mov   qword [sockfd], rax

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
  lea   rsi, [post_route]
  mov   rdx, post
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
  call  debug

  mov   rax, SYS_EXIT
  mov   rdi, FAILURE_CODE
  syscall


section .data
  sockfd  dq 0
  
  root_route    db "/", NULL_CHAR
  index_route   db "/index", NULL_CHAR
  health_route  db "/health", NULL_CHAR
  post_route    db "/post", NULL_CHAR

  ok  db "ok", NULL_CHAR

  index_path  db "views/index.html", NULL_CHAR
  post_path   db "views/post.html", NULL_CHAR


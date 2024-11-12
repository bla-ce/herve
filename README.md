# Implementation of a HTTP Server Library in Netwide Assembly

> [!WARNING]
> This library is for educational or experimental purposes only.
> It is NOT suitable yet for production use

## Official Documentation will be available soon

## Example

```asm
global  _start

%include "bytasm.inc"

section .text
print_hello:
  lea   rdi, [hello]
  mov   rsi, 0
  call  println

  ret

index:
  lea   rdi, [index_path]
  lea   rsi, [CONTENT_HTML]
  call  serve_static_file

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

  call  disable_log_color
  lea   rdi, [log_file]
  mov   rsi, O_WRONLY
  or    rsi, O_APPEND
  or    rsi, O_CREAT
  mov   rdx, S_IWUSR
  or    rdx, S_IRUSR
  or    rdx, S_IRGRP
  or    rdx, S_IROTH
  call  open_file
  cmp   rax, 0
  jl    error
  mov   rdi, rax
  call  set_log_output

  mov   rdi, print_hello
  mov   rsi, qword [sockfd]
  call  add_middleware

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

  lea   rdi, [views_dir]
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
  index_route   db "/index", NULL_CHAR

  views_dir   db "examples/views", NULL_CHAR
  index_path  db "examples/views/index.html", NULL_CHAR

  ok  db "ok", NULL_CHAR

  name_param db "name", NULL_CHAR

  hello db "Hello, World!", NULL_CHAR

  log_file db "app.log", NULL_CHAR
```

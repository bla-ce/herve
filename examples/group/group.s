global _start

%include "herve.inc"
%include "os.inc"

section .text

print_pre_root:
  mov   rdi, hello_pre_root
  call  println
  ret

print_pre_v1:
  mov   rdi, hello_pre_v1
  call  println
  ret

print_pre_v2:
  mov   rdi, hello_pre_v2
  call  println
  ret

print_post_root:
  mov   rdi, hello_post_root
  call  println
  ret

print_post_v1:
  mov   rdi, hello_post_v1
  call  println
  ret

print_post_v2:
  mov   rdi, hello_post_v2
  call  println
  ret

_start:
  sub   rsp, 0x18

  mov   rdi, 1337
  call  server_init
  cmp   rax, 0
  jl    .error

  mov   [rsp], rax 

  ; add /v1 group
  mov   rdi, [rsp]
  mov   rsi, v1_group
  mov   rdx, 0
  call  add_group
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; add /v2 group
  mov   rdi, [rsp]
  mov   rsi, v2_group
  mov   rdx, 0
  call  add_group
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; add pre middleware to root group
  mov   rdi, [rsp]
  mov   rsi, print_pre_root
  mov   rdx, 0
  mov   rcx, 0
  mov   r8, 0
  mov   r9, FALSE
  call  add_middleware
  cmp   rax, 0
  jl    .error

  ; add post middleware to root group
  mov   rdi, [rsp]
  mov   rsi, print_post_root
  mov   rdx, 0
  mov   rcx, 0
  mov   r8, 0
  mov   r9, TRUE
  call  add_middleware
  cmp   rax, 0
  jl    .error

  ; add pre middleware to v1 group
  mov   rdi, [rsp]
  mov   rsi, print_pre_v1
  mov   rdx, [rsp+0x8]
  mov   rcx, 0
  mov   r8, 0
  mov   r9, FALSE
  call  add_middleware
  cmp   rax, 0
  jl    .error

  ; add pre middleware to v2 group
  mov   rdi, [rsp]
  mov   rsi, print_pre_v2
  mov   rdx, [rsp+0x10]
  mov   rcx, 0
  mov   r8, 0
  mov   r9, FALSE
  call  add_middleware
  cmp   rax, 0
  jl    .error

  ; add post middleware to v1 group
  mov   rdi, [rsp]
  mov   rsi, print_post_v1
  mov   rdx, [rsp+0x8]
  mov   rcx, 0
  mov   r8, 0
  mov   r9, TRUE
  call  add_middleware
  cmp   rax, 0
  jl    .error

  ; add post middleware to v2 group
  mov   rdi, [rsp]
  mov   rsi, print_post_v2
  mov   rdx, [rsp+0x10]
  mov   rcx, 0
  mov   r8, 0
  mov   r9, TRUE
  call  add_middleware
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, GET
  mov   rdx, v1_url
  mov   rcx, 0
  mov   r8, [rsp+0x8]
  call  add_route
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, GET
  mov   rdx, v2_url
  mov   rcx, 0
  mov   r8, [rsp+0x10]
  call  add_route
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  call  run_server
  cmp   rax, 0
  jl    .error

  mov   rdi, SUCCESS_CODE
  call  exit

.error:
  mov   rdi, FAILURE_CODE
  call  exit
  
section .data
  v1_group      db "/v1", NULL_CHAR
  v1_url        db "/", NULL_CHAR
  hello_pre_v1  db "Hello, pre v1 group!", NULL_CHAR
  hello_post_v1 db "Hello, post v1 group!", NULL_CHAR

  v2_group      db "/v2", NULL_CHAR
  v2_url        db "/", NULL_CHAR
  hello_pre_v2  db "Hello, pre v2 group!", NULL_CHAR
  hello_post_v2 db "Hello, post v2 group!", NULL_CHAR

  root_group      db "/root", NULL_CHAR
  hello_pre_root  db "Hello, pre root group!", NULL_CHAR
  hello_post_root db "Hello, post root group!", NULL_CHAR


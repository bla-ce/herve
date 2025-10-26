global _start

%include "herve.inc"

section .text

; prints public message
public_handler:
  mov   rdi, public_url
  call  println
  ret

; prints internal message
internal_handler:
  mov   rdi, internal_url
  call  println
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

  ; create public group
  mov   rdi, [server]
  mov   rsi, public_url
  mov   rdx, FALSE
  call  add_group
  cmp   rax, 0
  jl    .error

  mov   [public_group], rax

  ; create middleware
  mov   rdi, public_handler
  xor   rsi, rsi
  xor   rdx, rdx
  xor   rcx, rcx
  mov   r9, FALSE
  call  create_middleware
  cmp   rax, 0
  jl    .error

  mov   rdi, [server]
  mov   rsi, [public_group]
  mov   rdx, rax
  call  add_middleware
  cmp   rax, 0
  jl    .error

  ; create internal group
  mov   rdi, [server]
  mov   rsi, internal_url
  mov   rdx, FALSE
  call  add_group
  cmp   rax, 0
  jl    .error

  mov   [internal_group], rax

  mov   rdi, internal_handler
  xor   rsi, rsi
  xor   rdx, rdx
  xor   rcx, rcx
  mov   r9, FALSE
  call  create_middleware
  cmp   rax, 0
  jl    .error

  mov   rdi, [server]
  mov   rsi, [internal_group]
  mov   rdx, rax
  call  add_middleware
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

  public_url    db "/public", NULL_CHAR
  internal_url  db "/internal", NULL_CHAR

  public_msg    db "This url is public", NULL_CHAR
  internal_msg  db "This url is internal, how did you get there?", NULL_CHAR

  public_group    dq 0
  internal_group  dq 0


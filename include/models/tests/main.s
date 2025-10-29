global _start

%include "herve.inc"
%include "models_test.s"

section .text
_start:
  sub   rsp, 0x18

  call  create_user_model
  cmp   rax, 0
  jl    .error

  mov   [rsp], rax

  mov   rdi, [rsp]
  call  model_instance_create
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  mov   rdx, field_username
  mov   rcx, user1
  call  model_instance_set
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  mov   rdx, field_password
  mov   rcx, pass1
  call  model_instance_set
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  mov   rdx, field_password
  call  model_instance_get
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, pass1
  call  strcmp
  cmp   rax, 0
  jl    .error

  cmp   rax, TRUE
  jne   .error

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  mov   rdx, pass1
  mov   rcx, pass1
  call  model_instance_set
  cmp   rax, 0
  jge   .error  ; should fail

  mov   rdi, [rsp]
  call  model_instance_create
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  mov   rdi, [rsp]
  call  free
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  call  free
  cmp   rax, 0
  jl    .error

  mov   rdi, SUCCESS_CODE
  call  exit

.error:
  mov   rdi, FAILURE_CODE
  call  exit

section .data
  user1 db "user1", NULL_CHAR
  user2 db "user2", NULL_CHAR

  pass1 db "pass1", NULL_CHAR
  pass2 db "pass2", NULL_CHAR

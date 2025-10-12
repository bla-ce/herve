global _start

%include "herve.inc"

section .text
_start:
  sub   rsp, 0x8

  mov   rdi, model_name
  call  model_create
  cmp   rax, 0
  jl    .error

  mov   [rsp], rax

  mov   rdi, [rsp]
  mov   rsi, field_username
  mov   rdx, FIELD_TYPE_STRING
  mov   rcx, 32
  call  model_insert_field
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, field_password
  mov   rdx, FIELD_TYPE_STRING
  mov   rcx, 128
  call  model_insert_field
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, field_age
  mov   rdx, FIELD_TYPE_NUMBER
  call  model_insert_field
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, field_active
  mov   rdx, FIELD_TYPE_BOOL
  call  model_insert_field
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  call  model_free
  cmp   rax, 0
  jl    .error

  mov   rdi, SUCCESS_CODE
  call  exit

.error:
  mov   rdi, FAILURE_CODE
  call  exit

section .data
  model_name  db "User", NULL_CHAR

  field_username db "username", NULL_CHAR
  field_password db "password", NULL_CHAR
  field_age      db "age", NULL_CHAR
  field_active   db "active", NULL_CHAR


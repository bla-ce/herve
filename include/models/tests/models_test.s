global _start

%include "herve.inc"

section .text
_start:
  mov   rdi, model_name
  call  model_create
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
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


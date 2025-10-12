global _start

%include "herve.inc"
%include "models.s"

section .text
_start:
  sub   rsp, 0x8

  call  create_user_model
  cmp   rax, 0
  jl    .error

  mov   [rsp], rax

  mov   rdi, [rsp]
  call  model_create_instance
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  call  model_create_instance
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


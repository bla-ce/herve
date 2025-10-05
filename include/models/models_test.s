global _start

%include "herve.inc"
%include "os.inc"

section .text

_start:
  sub   rsp, 0x8

  ; *** STACK USAGE *** ;
  ; [rsp] -> pointer to the model struct

  mov   rdi, model_name
  call  model_init
  cmp   rax, 0
  jl    .error

  mov   [rsp], rax

  mov   rdi, [rsp]
  call  model_get_name
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, model_name
  call  strcmp
  cmp   rax, 0
  jl    .error

  cmp   rax, TRUE
  jne   .error

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

model_name db "Person", NULL_CHAR

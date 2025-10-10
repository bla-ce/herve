global _start

%include "herve.inc"
%include "os.inc"

section .text

_start:
  mov   rdi, 10
  mov   rsi, 8
  call  array_create
  cmp   rax, 0
  jl    .error

  mov   rdi, SUCCESS_CODE
  call  exit

.error:
  mov   rdi, FAILURE_CODE
  call  exit
    

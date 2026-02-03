%include "lib.inc"
%include "herve.inc"

global _start

section .data

hello_world db "Hello, World, AWS :)", NULL_CHAR

section .text

_start:
  mov   rdi, hello_world
  call  println
  cmp   rax, 0
  jl    .error

  jmp  .exit

.error:
  mov   rdi, FAILURE_CODE
  call  exit

.exit:
  mov   rdi, SUCCESS_CODE
  call  exit

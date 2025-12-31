global  _start

%include "herve.inc"

section .data
  upper_string db "HELLO, WORLD! 9", NULL_CHAR
  lower_string db "hello, world! 9", NULL_CHAR

  mixed_string db "hELlo, WORld! 9", NULL_CHAR

section .text

_start:
  mov   rdi, upper_string
  call  to_lower
  cmp   rax, 0
  jl    .error

  mov   rdi, lower_string
  mov   rsi, rax
  call  strcmp
  cmp   rax, 0
  jle   .error

  mov   rdi, mixed_string
  call  to_lower
  cmp   rax, 0
  jl    .error

  mov   rdi, lower_string
  mov   rsi, rax
  call  strcmp
  cmp   rax, 0
  jle   .error

  mov   rdi, upper_string
  call  to_upper
  cmp   rax, 0
  jl    .error

  mov   rdi, upper_string
  mov   rsi, rax
  call  strcmp
  cmp   rax, 0
  jle   .error

  mov   rdi, lower_string
  call  to_upper
  cmp   rax, 0
  jl    .error

  mov   rdi, upper_string
  mov   rsi, rax
  call  strcmp
  cmp   rax, 0
  jle   .error

  mov   rdi, mixed_string
  call  to_upper
  cmp   rax, 0
  jl    .error

  mov   rdi, upper_string
  mov   rsi, rax
  call  strcmp
  cmp   rax, 0
  jle   .error

  mov   rdi, SUCCESS_CODE
  call  exit

.error:
  mov   rdi, FAILURE_CODE
  call  exit

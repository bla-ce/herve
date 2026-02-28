global  _start

%include "lib.inc"

section .data
  upper_string db "HELLO, WORLD! 9", NULL_CHAR
  lower_string db "hello, world! 9", NULL_CHAR

  mixed_string db "hELlo, WORld! 9", NULL_CHAR

  str_1 db "Hello, world!", NULL_CHAR
  str_2 db "Hello, sir!", NULL_CHAR

section .text

_start:
  ; to_lower and to_upper test
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

  ; constant time strcmp tests
  mov   rdi, str_1
  mov   rsi, str_2
  call  strcmp_const_time

  mov   rdi, rax
  call  assert_false

  mov   rdi, str_1
  mov   rsi, str_1
  call  strcmp_const_time

  mov   rdi, rax
  call  assert_true

  mov   rdi, str_1
  mov   rsi, str_2
  mov   rdx, 4
  call  strncmp_const_time

  mov   rdi, rax
  call  assert_true

  mov   rdi, str_1
  mov   rsi, str_2
  mov   rdx, 8
  call  strncmp_const_time

  mov   rdi, rax
  call  assert_false

  mov   rdi, SUCCESS_CODE
  call  exit

.error:
  mov   rdi, FAILURE_CODE
  call  exit

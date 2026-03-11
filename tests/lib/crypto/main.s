global _start

%include "lib.inc"

section .text
_start:

.loop:
  mov   rdi, password
  mov   rsi, 12
  call  bcrypt_hash
  cmp   rax, 0
  jl    .error

  mov   [hash], rax

  mov   rdi, rax
  call  println
  cmp   rax, 0
  jl    .error

  mov   rdi, [hash]
  call  free
  cmp   rax, 0
  jl    .error

  dec   qword [counter]
  cmp   qword [counter], 0
  jle   .loop_end

  jmp   .loop

.loop_end:
  mov   rdi, qword [mallocd]
  mov   rsi, qword [freed]
  call  assert_equal

  mov   rdi, SUCCESS_CODE
  call  exit

.error:
  mov   rdi, FAILURE_CODE
  call  exit

section .data

password db "hello, sir", NULL_CHAR

hash dq 0

counter dq 10

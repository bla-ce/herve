global _start

%include "lib.inc"

section .text
_start:
.loop:
  call  uuid_v4
  cmp   rax, 0
  jl    .error

  mov   [uuid], rax

  mov   rdi, [uuid]
  call  println

  mov   rdi, [uuid]
  call  free

  dec   qword [counter]
  cmp   qword [counter], 0
  jle   .loop_end

  jmp   .loop

.loop_end:
  mov   rax, [mallocd]
  cmp   rax, [freed]
  jne   .error

  mov   rdi, SUCCESS_CODE
  call  exit

.error:
  mov   rdi, FAILURE_CODE
  call  exit

section .data

counter dq 500
uuid    dq 0

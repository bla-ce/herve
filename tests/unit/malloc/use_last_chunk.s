global _start

%include "malloc_utils.inc"
%include "malloc.inc"

section .text
_start:
  mov   rdi, 12800
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [p1], rax

  mov   rdi, rax
  mov   rax, 12800
  mov   rcx, 12800
  rep   stosb

  mov   rdi, [p1]
  call  free
  cmp   rax, 0
  jl    .error

  mov   rdi, 12800
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [p2], rax

  mov   rdi, rax
  call  free
  cmp   rax, 0
  jl    .error

.exit:
  mov   rax, SYS_EXIT
  mov   rdi, 0
  syscall

.error:
  mov   rax, SYS_EXIT
  mov   rdi, FAILURE_CODE
  syscall

section .data
  p1    dq 0
  p2    dq 0

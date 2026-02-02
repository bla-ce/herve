global _start

%include "malloc_utils.inc"
%include "malloc.inc"

section .text
_start:
  mov   rdi, 1024
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [p1], rax

  mov   rdi, rax

  mov   rax, 0xFE
  mov   rcx, 1024
  rep   stosb

  mov   rdi, [p1]
  call  free
  cmp   rax, 0
  jl    .error

  mov   rdi, 12
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [p2], rax

  mov   rdi, rax
  mov   rax, 0x4
  mov   rcx, 12
  rep   stosb

  mov   rdi, 64
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [p3], rax

  mov   rdi, rax
  mov   rax, 0x4
  mov   rcx, 64
  rep   stosb

  mov   rdi, [p3]
  call  free
  cmp   rax, 0
  jl    .error

  mov   rdi, 24
  call  malloc
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
  p3    dq 0

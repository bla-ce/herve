global _start

%include "malloc.inc"
%include "malloc_utils.inc"

section .text
_start:
  mov   rdi, 16
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   [p1], rax

  mov   rdi, 16
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   [p2], rax

  mov   rdi, 32
  call  malloc

  mov   rdi, [p2]
  call  free

  mov   rdi, [p1]
  call  free

  mov   rax, [p1]
  sub   rax, CHUNK_METADATA_LEN

  cmp   qword [rax+CHUNK_OFFSET_SIZE], 32+2*CHUNK_METADATA_LEN
  jne   .error

.exit:
  mov   rax, 60
  mov   rdi, 0
  syscall

.error:
  mov   rax, 60
  mov   rdi, -1
  syscall

section .data
  p1    dq 0
  p2    dq 0


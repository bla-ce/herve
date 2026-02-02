global _start

%include "malloc_utils.inc"
%include "malloc.inc"

section .text
_start:
  mov   rdi, 16
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [p1], rax

  sub   rax, CHUNK_METADATA_LEN

  cmp   qword [rax+CHUNK_OFFSET_SIZE], 16+CHUNK_METADATA_LEN
  jne   .error

  mov   rdi, 32
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [p2], rax

  mov   rbx, [p1]
  cmp   rax, rbx
  je    .error

  sub   rax, CHUNK_METADATA_LEN

  cmp   qword [rax+CHUNK_OFFSET_SIZE], 32+CHUNK_METADATA_LEN
  jne   .error

  mov   rdi, 64
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [p3], rax

  mov   rbx, [p2]
  cmp   rax, rbx
  je    .error

  sub   rax, CHUNK_METADATA_LEN

  cmp   qword [rax+CHUNK_OFFSET_SIZE], 64+CHUNK_METADATA_LEN
  jne   .error

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

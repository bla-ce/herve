global _start

%include "malloc_utils.inc"
%include "malloc.inc"

section .text
_start:
  mov   rdi, 128
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   [p1], rax

  mov   rdi, [p1]
  call  free

  mov   rdi, 64
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   [p2], rax

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


global _start

%include "malloc_utils.inc"
%include "malloc.inc"

section .text
_start:
  ; [ /56, 4040 ]
  mov   rdi, 16
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [p1], rax

  ; [ /56, /72, 3968 ]
  mov   rdi,  32
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [p2], rax

  ; [ /56, /72, /104, 3864 ]
  mov   rdi,  64
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [p3], rax

  ; [ 56, /72, /104, 3864 ]
  mov   rdi, [p1]
  call  free
  cmp   rax, 0
  jl    .error

  mov   rax, [p1]
  sub   rax, CHUNK_METADATA_LEN
  cmp   qword [rax+CHUNK_OFFSET_INUSE], 0
  jne   .error

  ; [ 56, 72, /104, 3864 ]
  mov   rdi, [p2]
  call  free
  cmp   rax, 0
  jl    .error

  mov   rax, [p2]
  sub   rax, CHUNK_METADATA_LEN
  cmp   qword [rax+CHUNK_OFFSET_INUSE], 0
  jne   .error

  ; [ /56, 72, /104, 3864 ]
  mov   rdi, 16
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [p4], rax

  sub   rax, CHUNK_METADATA_LEN
  cmp   qword [rax+CHUNK_OFFSET_INUSE], 1
  cmp   qword [rax+CHUNK_OFFSET_SIZE], 56
  jne   .error

  mov   rax, [p4]
  mov   rbx, [p1]
  cmp   rax, rbx
  jne   .error

  ; [ 128, /104, 3864 ]
  mov   rdi, [p4]
  call  free
  cmp   rax, 0
  jl    .error

  mov   rax, [p4]
  sub   rax, CHUNK_METADATA_LEN
  cmp   qword [rax+CHUNK_OFFSET_INUSE], 0
  jne   .error

  cmp   qword [rax+CHUNK_OFFSET_SIZE], 128
  jne   .error

  ; [ /128, /104, 3864 ]
  mov   rdi, 88
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [p6], rax

  sub   rax, CHUNK_METADATA_LEN
  cmp   qword [rax+CHUNK_OFFSET_INUSE], 1
  jne   .error

  cmp   qword [rax+CHUNK_OFFSET_SIZE], 128
  jne   .error

  mov   rax, [p1]
  mov   rbx, [p4]
  mov   rdx, [p6]

  cmp   rax, rbx
  jne   .error

  cmp   rax, rdx
  jne   .error

  ; [ /128, /104, /3864 ]
  mov   rdi, 3824
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [p5], rax

  xor   rax, rax

.loop:
  cmp   qword [seg_free_list+0x8*rax], 0
  jne   .error

  cmp   rax, N_BINS-1
  je    .loop_end

  inc   rax
  jmp   .loop

.loop_end:
  ; [ /128, /104, /3864, /4040 ]
  mov   rdi, 4000
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
  p4    dq 0
  p5    dq 0
  p6    dq 0

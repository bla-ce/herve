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

  ; seg free list should be empty except 8th
  xor   rax, rax

.loop1:
  cmp   rax, 0x8
  je    .jump1

  cmp   qword [seg_free_list+0x8*rax], 0
  jne   .error

  cmp   rax, N_BINS-1
  je    .end_loop1

.jump1:
  inc   rax
  jmp   .loop1
.end_loop1:
  ; 8th bin should contain a 4040 bytes chunk with null pointers
  mov   rdi, qword [seg_free_list+0x8*0x8]
  cmp   rdi, 0
  je    .error

  cmp   qword [rdi+CHUNK_OFFSET_SIZE], 4040
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_PREV], 0
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_NEXT], 0
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_INUSE], 0
  jne   .error

  ; [ /56, /72, 3968 ]
  mov   rdi,  32
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   [p2], rax

  ; seg free list should be empty except 8th
  xor   rax, rax

.loop2:
  cmp   rax, 0x8
  je    .jump2

  cmp   qword [seg_free_list+0x8*rax], 0
  jne   .error

  cmp   rax, N_BINS-1
  je    .end_loop2

.jump2:
  inc   rax
  jmp   .loop2
.end_loop2:
  ; 8th bin should contain a 3968 bytes chunk with null pointers
  mov   rdi, qword [seg_free_list+0x8*0x8]
  cmp   rdi, 0
  je    .error

  cmp   qword [rdi+CHUNK_OFFSET_SIZE], 3968
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_PREV], 0
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_NEXT], 0
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_INUSE], 0
  jne   .error

  ; [ /56, /72, /104, 3864 ]
  mov   rdi,  64
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   [p3], rax

  ; seg free list should be empty except 8th
  xor   rax, rax

.loop3:
  cmp   rax, 0x8
  je    .jump3

  cmp   qword [seg_free_list+0x8*rax], 0
  jne   .error

  cmp   rax, N_BINS-1
  je    .end_loop3

.jump3:
  inc   rax
  jmp   .loop3
.end_loop3:
  ; 8th bin should contain a 3864 bytes chunk with null pointers
  mov   rdi, qword [seg_free_list+0x8*0x8]
  cmp   rdi, 0
  je    .error

  cmp   qword [rdi+CHUNK_OFFSET_SIZE], 3864
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_PREV], 0
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_NEXT], 0
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_INUSE], 0
  jne   .error

  ; [ 56, /72, /104, 3864 ]
  mov   rdi, [p1]
  call  free

  ; seg free list should be empty except 8th and 1st
  xor   rax, rax

.loop4:
  cmp   rax, 0x8
  je    .jump4

  cmp   rax, 0x1
  je    .jump4

  cmp   qword [seg_free_list+0x8*rax], 0
  jne   .error

  cmp   rax, N_BINS-1
  je    .end_loop4

.jump4:
  inc   rax
  jmp   .loop4
.end_loop4:
  ; 8th bin should contain a 3864 bytes chunk with null pointers
  mov   rdi, qword [seg_free_list+0x8*0x8]
  cmp   rdi, 0
  je    .error

  cmp   qword [rdi+CHUNK_OFFSET_SIZE], 3864
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_PREV], 0
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_NEXT], 0
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_INUSE], 0
  jne   .error

  ; 1st bin should contain a 56 bytes chunk with null pointers
  mov   rdi, qword [seg_free_list+0x8*0x1]
  cmp   rdi, 0
  je    .error

  cmp   qword [rdi+CHUNK_OFFSET_SIZE], 56
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_PREV], 0
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_NEXT], 0
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_INUSE], 0
  jne   .error

  mov   rax, [p1]
  sub   rax, CHUNK_METADATA_LEN
  cmp   qword [rax+CHUNK_OFFSET_INUSE], 0
  jne   .error

  ; [ 56, 72, /104, 3864 ]
  mov   rdi, [p2]
  call  free

  mov   rax, [p2]
  sub   rax, CHUNK_METADATA_LEN
  cmp   qword [rax+CHUNK_OFFSET_INUSE], 0
  jne   .error

  ; seg free list should be empty except 1st, 2nd and 8th
  xor   rax, rax

.loop5:
  cmp   rax, 0x1
  je    .jump5

  cmp   rax, 0x2
  je    .jump5

  cmp   rax, 0x8
  je    .jump5

  cmp   qword [seg_free_list+0x8*rax], 0
  jne   .error

  cmp   rax, N_BINS-1
  je    .end_loop5

.jump5:
  inc   rax
  jmp   .loop5
.end_loop5:
  ; 1st bin should contain a 56 bytes chunk with null pointers
  mov   rdi, qword [seg_free_list+0x8*0x1]
  cmp   rdi, 0
  je    .error

  cmp   qword [rdi+CHUNK_OFFSET_SIZE], 56
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_PREV], 0
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_NEXT], 0
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_INUSE], 0
  jne   .error

  ; 2nd bin should contain a 72 bytes chunk with null pointers
  mov   rdi, qword [seg_free_list+0x8*0x2]
  cmp   rdi, 0
  je    .error

  cmp   qword [rdi+CHUNK_OFFSET_SIZE], 72
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_PREV], 0
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_NEXT], 0
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_INUSE], 0
  jne   .error

  ; 8th bin should contain a 3864 bytes chunk with null pointers
  mov   rdi, qword [seg_free_list+0x8*0x8]
  cmp   rdi, 0
  je    .error

  cmp   qword [rdi+CHUNK_OFFSET_SIZE], 3864
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_PREV], 0
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_NEXT], 0
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_INUSE], 0
  jne   .error

  ; [ /56, 72, /104, 3864 ]
  mov   rdi, 16
  call  malloc

  mov   [p4], rax

  sub   rax, CHUNK_METADATA_LEN
  cmp   qword [rax+CHUNK_OFFSET_INUSE], 1
  cmp   qword [rax+CHUNK_OFFSET_SIZE], 56
  jne   .error

  mov   rax, [p4]
  mov   rbx, [p1]
  cmp   rax, rbx
  jne   .error

  ; seg free list should be empty except 2nd and 8th
  xor   rax, rax

.loop6:
  cmp   rax, 0x2
  je    .jump6

  cmp   rax, 0x8
  je    .jump6

  cmp   qword [seg_free_list+0x8*rax], 0
  jne   .error

  cmp   rax, N_BINS-1
  je    .end_loop6

.jump6:
  inc   rax
  jmp   .loop6
.end_loop6:
  ; 2nd bin should contain a 72 bytes chunk with null pointers
  mov   rdi, qword [seg_free_list+0x8*0x2]
  cmp   rdi, 0
  je    .error

  cmp   qword [rdi+CHUNK_OFFSET_SIZE], 72
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_PREV], 0
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_NEXT], 0
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_INUSE], 0
  jne   .error

  ; 8th bin should contain a 3864 bytes chunk with null pointers
  mov   rdi, qword [seg_free_list+0x8*0x8]
  cmp   rdi, 0
  je    .error

  cmp   qword [rdi+CHUNK_OFFSET_SIZE], 3864
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_PREV], 0
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_NEXT], 0
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_INUSE], 0
  jne   .error

  ; [ 128, /104, 3864 ]
  mov   rdi, [p4]
  call  free

  mov   rax, [p4]
  sub   rax, CHUNK_METADATA_LEN
  cmp   qword [rax+CHUNK_OFFSET_INUSE], 0
  jne   .error

  cmp   qword [rax+CHUNK_OFFSET_SIZE], 128
  jne   .error

  ; seg free list should be empty except 3rd and 8th
  xor   rax, rax

.loop7:
  cmp   rax, 0x3
  je    .jump7

  cmp   rax, 0x8
  je    .jump7

  cmp   qword [seg_free_list+0x8*rax], 0
  jne   .error

  cmp   rax, N_BINS-1
  je    .end_loop7

.jump7:
  inc   rax
  jmp   .loop7
.end_loop7:
  ; 3rd bin should contain a 128 bytes chunk with null pointers
  mov   rdi, qword [seg_free_list+0x8*0x3]
  cmp   rdi, 0
  je    .error

  cmp   qword [rdi+CHUNK_OFFSET_SIZE], 128
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_PREV], 0
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_NEXT], 0
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_INUSE], 0
  jne   .error

  ; 8th bin should contain a 3864 bytes chunk with null pointers
  mov   rdi, qword [seg_free_list+0x8*0x8]
  cmp   rdi, 0
  je    .error

  cmp   qword [rdi+CHUNK_OFFSET_SIZE], 3864
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_PREV], 0
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_NEXT], 0
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_INUSE], 0
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

  ; seg free list should be empty except 8th
  xor   rax, rax

.loop8:
  cmp   rax, 0x8
  je    .jump8

  cmp   qword [seg_free_list+0x8*rax], 0
  jne   .error

  cmp   rax, N_BINS-1
  je    .end_loop8

.jump8:
  inc   rax
  jmp   .loop8
.end_loop8:
  ; 8th bin should contain a 3864 bytes chunk with null pointers
  mov   rdi, qword [seg_free_list+0x8*0x8]
  cmp   rdi, 0
  je    .error

  cmp   qword [rdi+CHUNK_OFFSET_SIZE], 3864
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_PREV], 0
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_NEXT], 0
  jne   .error

  cmp   qword [rdi+CHUNK_OFFSET_INUSE], 0
  jne   .error

  ; [ /128, /104, /3864 ]
  mov   rdi, 3824
  call  malloc

  cmp   rax, 0
  jl    .error

  xor   rax, rax

.loop:
  cmp   qword [seg_free_list+0x8*rax], 0
  jne   .error

  cmp   rax, N_BINS-1
  je    .end_loop

  inc   rax
  jmp   .loop

.end_loop:
  ; [ /128, /104, /3864, /4040 ]
  mov   rdi, 4000
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   [p7], rax

  mov   rdi, rax
  call  free

  mov   rdi, 400
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
  p7    dq 0


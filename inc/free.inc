section .text
; checks if adjacent chunks can be merged together
; @param  rdi: chunk
; @return rax: bool
can_coalesce:
  mov   rsi, qword [rdi+CHUNK_OFFSET_SIZE]
  add   rdi, rsi

  cmp   qword [rdi+CHUNK_OFFSET_INUSE], 0
  sete  al
  movzx rax, al
  
  ret

; coalesces adjacent chunks
; @param  rdi: chunk
; @return rax: chunk
coalesce:
  sub   rsp, 0x10

  ; save both chunks onto the stack
  mov   [rsp], rdi
  mov   rsi, rdi
  add   rdi, qword [rdi+CHUNK_OFFSET_SIZE]
  mov   [rsp+0x8], rdi

  ; update first chunk size
  mov   rax, qword [rdi+CHUNK_OFFSET_SIZE]
  add   qword [rsi+CHUNK_OFFSET_SIZE], rax

  ; remove second chunk from the free_list
  mov   rax, qword [rdi+CHUNK_OFFSET_NEXT]
  mov   qword [rsi+CHUNK_OFFSET_NEXT], rax

  ; return first chunk
  mov   rax, [rsp]

  add   rsp, 0x10
  ret

; returns the index of the corresponding bin
; @param  rdi: size
; @return rax: bin index
get_bin:
  bsr   rax, rdi

  ; free list is 0-indexed
  mov   rbx, N_BINS - 1
  sub   rax, FIRST_BIN_BSR_INDEX

  cmp   rax, rbx
  cmovge rax, rbx

  mov   rbx, 0
  cmp   rax, rbx
  cmovl rax, rbx

  ret

; adds a chunk of memory to the free list
; @param  rdi: chunk of memory to add
; @return rax: void
add_to_free_list:
  sub   rsp, 0x10

  mov   [rsp], rdi

  ; make sure the heap is not corrupted
  cmp   qword [rdi+CHUNK_OFFSET_MAGIC], MAGIC_VAL_HEAP_CORRUPT
  jne   .error

  mov   rdi, qword [rdi+CHUNK_OFFSET_SIZE]
  call  get_bin
  
  ; save bin index
  mov   qword [rsp+0x8], rax
  
  lea   rsi, [seg_free_list+0x8*rax]
  cmp   qword [rsi], 0
  je    .init_free_list
  
  mov   rdi, [rsp]
  mov   rbx, [rsi]
  
  mov   qword [rdi+CHUNK_OFFSET_NEXT], rbx
  mov   qword [rbx+CHUNK_OFFSET_PREV], rdi
  
  ; update free_list top - LIFO
  mov   rax, qword [rsp+0x8] 
  mov   [seg_free_list+0x8*rax], rdi

  xor   rax, rax
  
  jmp   .return
  
.init_free_list:
  mov   rdi, [rsp]
  mov   [seg_free_list+0x8*rax], rdi

  xor   rax, rax

.return:
  add   rsp, 0x10
  ret

.error:
  mov   rax, FAILURE_CODE
  jmp   .return

; frees previously allocated memory
; @param  rdi: pointer to chunk data
; @return rax: void
free:
  sub   rsp, 0x8
  
  ; go to the beginning of the struct
  sub   rdi, CHUNK_SIZE

  mov   [rsp], rdi

  ; check if adjacent chunk is free for coalescing
  call  can_coalesce

  cmp   rax, 0
  je    .free_chunk

  mov   rdi, [rsp]
  call  coalesce

  mov   [rsp], rax

.free_chunk:
  mov   rdi, [rsp]

  ; chunk not in use
  mov   qword [rdi+CHUNK_OFFSET_INUSE], 0
  call  add_to_free_list

  add   rsp, 0x8
  ret


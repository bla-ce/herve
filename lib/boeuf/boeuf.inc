section .bss

boeuf_t:
  .magic_value  resq 1
  .size         resq 1
  .length       resq 1
  .data         resq 1
boeuf_t_end:

section .data

BOEUF_MAGIC_VALUE equ 0xC0FFEE

BOEUF_LEN equ boeuf_t_end - boeuf_t

; offsets
BOEUF_OFF_MAGIC_VALUE  equ 0
BOEUF_OFF_SIZE         equ BOEUF_OFF_MAGIC_VALUE + 0x8
BOEUF_OFF_LENGTH       equ BOEUF_OFF_SIZE + 0x8
BOEUF_OFF_DATA         equ BOEUF_OFF_LENGTH + 0x8

BOEUF_DEFAULT_MAX_SIZE equ 1000000 ; 1MB

boeuf_max_size dq BOEUF_DEFAULT_MAX_SIZE

section .text
; sets the new limit for a buffer
; @param  rdi: new size
boeuf_set_limit:
  mov   [boeuf_max_size], rdi
  ret


; sets the original limit for a buffer
boeuf_reset_limit:
  mov   qword [boeuf_max_size], BOEUF_DEFAULT_MAX_SIZE
  ret

; creates a growing buffer, specifying the number of bytes to copy
; @param  rdi: pointer to the string
; @param  rsi: n bytes
; @return rax: pointer to the buffer
boeuf_ncreate:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> pointer to the string
  ; [rsp+0x8]   -> length of the string
  ; [rsp+0x10]  -> pointer to the boeuf buffer

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  cmp   rsi, qword [boeuf_max_size]
  jge   .error

  ; add metadata struct length
  mov   rdi, rsi
  add   rdi, BOEUF_METADATA_LEN
  inc   rdi  ; add one for null_char

  call  malloc
  cmp   rax, 0
  jl    .error

  ; init metadata
  mov   qword [rax+BOEUF_OFF_MAGIC_VALUE], BOEUF_MAGIC_VALUE

  mov   rdx, qword [rsp+0x8]
  inc   rdx ; add the null char

  mov   [rax+BOEUF_OFF_SIZE], rdx
  mov   [rax+BOEUF_OFF_LENGTH], rdx
  mov   qword [rax+BOEUF_OFF_DATA], 0

  ; return pointer to the data
  add   rax, BOEUF_OFF_DATA

  ; save pointer to the data
  mov   [rsp+0x10], rax

  cmp   qword [rsp+0x8], 0
  je    .skip_copy

  ; copy string to the boeuf buffer
  mov   rdi, rax
  mov   rsi, [rsp]
  mov   rcx, qword [rsp+0x8]
  rep   movsb

  ; add null char
  mov   rax, NULL_CHAR
  stosb

.skip_copy:
  mov   rax, [rsp+0x10]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; creates a growing buffer
; @param  rdi: pointer to the string
; @return rax: pointer to the buffer
boeuf_create:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp]       -> pointer to the string

  mov   [rsp], rdi

  test  rdi, rdi
  jz    .error

  ; get length of the string
  call  strlen
  cmp   rax, 0
  jle   .error

  ; save length of the string
  mov   rdi, [rsp]
  mov   rsi, rax
  call  boeuf_ncreate
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; appends n bytes from string to a buffer
; @param  rdi: pointer to the boeuf buffer
; @param  rsi: pointer to the string to be appended
; @param  rdx: n bytes to append
; @return rax: return code
boeuf_nappend:
  sub   rsp, 0x48

  ; STACK USAGE
  ; [rsp]       -> pointer to the boeuf buffer
  ; [rsp+0x8]   -> pointer to the string
  ; [rsp+0x10]  -> number of bytes to append
  ; [rsp+0x18]  -> pointer to new boeuf buffer
  ; [rsp+0x20]  -> old size of boeuf
  ; [rsp+0x28]  -> old length of boeuf
  ; [rsp+0x30]  -> length of the string
  ; [rsp+0x38]  -> new length of the boeuf
  ; [rsp+0x40]  -> new size of the boeuf

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   [rsp+0x18], rdi

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  ; check if we need to grow the boeuf buffer
  ; get length and size of current boeuf buffer
  mov   rdi, [rsp]

  mov   rax, [rdi-BOEUF_METADATA_LEN+BOEUF_OFF_SIZE]
  mov   qword [rsp+0x20], rax
  mov   qword [rsp+0x40], rax

  mov   rax, [rdi-BOEUF_METADATA_LEN+BOEUF_OFF_LENGTH]
  mov   qword [rsp+0x28], rax
  mov   qword [rsp+0x38], rax

  ; get length of the string
  mov   rdi, [rsp+0x8]
  call  strlen
  cmp   rax, 0
  jl    .error

  ; if bytes to append > len of the string -> error
  cmp   rax, qword [rsp+0x10]
  jl    .error

  ; if boeuf.size < boeuf.len + str.len -> realloc boeuf
  mov   rdi, [rsp+0x28]
  add   rdi, [rsp+0x10] ; new length
  mov   [rsp+0x38], rdi

  cmp   rdi, qword [boeuf_max_size]
  jge   .error

  cmp   rdi, qword [rsp+0x20]
  jle   .copy

  ; realloc boeuf buffer
  ; double the size until its greater than new size
  mov   rax, [rsp+0x20]

.loop:
  shl   rax, 1

  cmp   rax, [rsp+0x38]
  jl    .loop
.loop_end:

  mov   qword [rsp+0x40], rax

  ; add size of the boeuf metadata len
  add   rax, BOEUF_METADATA_LEN

  mov   rdi, [rsp]
  sub   rdi, BOEUF_METADATA_LEN
  mov   rsi, rax
  call  realloc
  cmp   rax, 0
  jl    .error

  add   rax, BOEUF_METADATA_LEN
  mov   [rsp+0x18], rax

.copy:
  ; get boeuf buffer
  mov   rdi, [rsp+0x18]

  ; go after string
  add   rdi, qword [rsp+0x28]
  dec   rdi  ; decrement to go before null char

  ; copy string
  mov   rsi, [rsp+0x8]
  mov   rcx, qword [rsp+0x10]
  rep   movsb

  ; add null char
  mov   rax, NULL_CHAR
  stosb

  mov   rax, [rsp+0x18]
  sub   rax, BOEUF_METADATA_LEN

  ; update size and length of the boeuf
  mov   rdx, [rsp+0x38]
  mov   qword [rax+BOEUF_OFF_LENGTH], rdx

  mov   rdx, [rsp+0x40]
  mov   qword [rax+BOEUF_OFF_SIZE], rdx

  mov   rax, [rsp+0x18]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x48
  ret

; appends string to a buffer
; @param  rdi: pointer to the boeuf buffer
; @param  rsi: pointer to the string to be appended
; @return rax: return code
boeuf_append:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]       -> pointer to the boeuf buffer
  ; [rsp+0x8]   -> pointer to the string

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  ; get length of string to be appended
  mov   rdi, [rsp+0x8]
  call  strlen
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  mov   rdx, rax
  call  boeuf_nappend
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; frees the boeuf buffer
; @param  rdi: pointer to the boeuf buffer to freed
; @return rax: return code
boeuf_free:
  sub   rdi, BOEUF_METADATA_LEN

  ; make sure of the integrity of the boeuf buffer
  cmp   qword [rdi+BOEUF_OFF_MAGIC_VALUE], BOEUF_MAGIC_VALUE
  jne   .error

  call  free
  cmp   rax, 0
  jl    .error

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; resets the boeuf buffer
; @param  rdi: pointer to the boeuf buffer
; @return rax: return code
boeuf_reset:
  test  rdi, rdi
  jz    .error

  ; add null char at the beginning of the string
  mov   rax, NULL_CHAR
  stosb

  dec   rdi
  sub   rdi, BOEUF_METADATA_LEN

  ; make sure of the integrity of the boeuf buffer
  cmp   qword [rdi+BOEUF_OFF_MAGIC_VALUE], BOEUF_MAGIC_VALUE
  jne   .error

  ; set length to 0
  mov   qword [rdi+BOEUF_OFF_LENGTH], 0

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the length of the boeuf buffer
; @param  rdi: pointer to the boeuf buffer
; @return rax: length of the boeuf buffer
boeuf_len:
  test  rdi, rdi
  jz    .error

  sub   rdi, BOEUF_METADATA_LEN

  ; make sure of the integrity of the boeuf buffer
  cmp   qword [rdi+BOEUF_OFF_MAGIC_VALUE], BOEUF_MAGIC_VALUE
  jne   .error

  mov   rax, qword [rdi+BOEUF_OFF_LENGTH]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the new (smaller) length of the boeuf buffer
; @param  rdi: pointer to the boeuf buffer
; @param  rsi: new size
; @return rax: return code
boeuf_set_len:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> pointer to the boeuf buffer
  ; [rsp+0x8] -> new size of the boeuf buffer

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  sub   rdi, BOEUF_METADATA_LEN

  ; make sure of the integrity of the boeuf buffer
  cmp   qword [rdi+BOEUF_OFF_MAGIC_VALUE], BOEUF_MAGIC_VALUE
  jne   .error

  cmp   rsi, qword [rdi+BOEUF_OFF_LENGTH]
  jge   .return

  mov   qword [rdi+BOEUF_OFF_LENGTH], rsi
  add   rdi, BOEUF_METADATA_LEN
  add   rdi, rsi

  mov   rax, NULL_CHAR
  stosb

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

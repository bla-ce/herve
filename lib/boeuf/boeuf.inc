%include "boeuf_errors.inc"

section .bss

boeuf_metadata_t:
  .boeuf_magic_value  resq 1
  .boeuf_size         resq 1
  .boeuf_length       resq 1
boeuf_metadata_t_end:

section .data

BOEUF_MAGIC_VALUE dq 0xC0FFEE

BOEUF_METADATA_LEN equ boeuf_metadata_t_end - boeuf_metadata_t

; offsets
BOEUF_METADATA_OFF_MAGIC_VALUE  equ 0
BOEUF_METADATA_OFF_SIZE         equ BOEUF_METADATA_OFF_MAGIC_VALUE + 0x8
BOEUF_METADATA_OFF_LENGTH       equ BOEUF_METADATA_OFF_SIZE + 0x8
BOEUF_METADATA_OFF_DATA         equ BOEUF_METADATA_OFF_LENGTH + 0x8

MAX_BOEUF_BUF_SIZE equ 1000000 ; 1MB

section .text
; creates a growing buffer, specifying the number of bytes to copy
; @param  rdi: pointer to the string
; @param  rsi: n bytes
; @return rax: pointer to the buffer
boeuf_ncreate:
  sub   rsp, 0x18

  mov   qword [boeuf_errno], BOEUF_INVALID_ARG
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the string
  ; [rsp+0x8]   -> length of the string
  ; [rsp+0x10]  -> pointer to the boeuf buffer

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  ; add metadata struct length
  mov   rdi, rsi
  add   rdi, BOEUF_METADATA_LEN
  inc   rdi   ; add one for null_char

  call  malloc

  mov   qword [boeuf_errno], BOEUF_MEMORY_ERROR
  cmp   rax, 0
  jl    .error

  ; init metadata
  mov   rbx, qword [BOEUF_MAGIC_VALUE]
  mov   qword [rax+BOEUF_METADATA_OFF_MAGIC_VALUE], rbx

  mov   rbx, qword [rsp+0x8]
  inc   rbx ; add the null char
  mov   qword [rax+BOEUF_METADATA_OFF_SIZE], rbx

  mov   qword [rax+BOEUF_METADATA_OFF_LENGTH], rbx

  ; return pointer to the data
  add   rax, BOEUF_METADATA_OFF_DATA

  mov   [rsp+0x10], rax

  ; copy string to the boeuf buffer
  mov   rdi, rax
  mov   rsi, [rsp]
  mov   rcx, qword [rsp+0x8]
  rep   movsb

  ; add null char
  mov   rax, NULL_CHAR
  stosb

  mov   qword [boeuf_errno], BOEUF_NO_ERR

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

  mov   qword [boeuf_errno], BOEUF_INVALID_ARG
  cmp   rdi, 0
  jle   .error

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the string

  mov   [rsp], rdi

  ; get length of the string
  call  strlen

  mov   qword [boeuf_errno], BOEUF_INVALID_ARG
  cmp   rax, 0
  jle   .error

  ; save length of the string
  mov   rdi, [rsp]
  mov   rsi, rax
  call  boeuf_ncreate

  mov   qword [boeuf_errno], BOEUF_NO_ERR
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
  sub   rsp, 0x30

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the boeuf buffer
  ; [rsp+0x8]   -> pointer to the string
  ; [rsp+0x10]  -> pointer to boeuf metadata
  ; [rsp+0x18]  -> length of the string
  ; [rsp+0x20]  -> size of new boeuf buffer if reallocated
  ; [rsp+0x28]  -> pointer to new boeuf buffer if reallocated

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x18], rdx

  mov   qword [boeuf_errno], BOEUF_INVALID_ARG
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  cmp   rdx, 0
  jle   .error

  ; make sure of the integrity of the boeuf buffer
  sub   rdi, BOEUF_METADATA_LEN
  mov   rbx, qword [BOEUF_MAGIC_VALUE]

  mov   qword [boeuf_errno], BOEUF_CORRUPTED_BUF
  cmp   qword [rdi+BOEUF_METADATA_OFF_MAGIC_VALUE], rbx
  jne   .error

  ; get pointer to the boeuf struct
  mov   rdi, [rsp]
  sub   rdi, BOEUF_METADATA_LEN
  mov   [rsp+0x10], rdi

  ; check if boeuf needs to grow
  mov   rax, qword [rdi+BOEUF_METADATA_OFF_LENGTH]
  mov   rbx, qword [rdi+BOEUF_METADATA_OFF_SIZE]
  mov   rcx, qword [rsp+0x18] ; length of the string

  ; add to current length of the boeuf buffer, the length of the string to be appended
  add   rax, rcx

  ; compare it with the size of the boeuf buffer
  cmp   rax, rbx
  jl    .no_grow

  ; save new size onto the stack
  mov   qword [rsp+0x20], rax

  ; malloc new boeuf buffer
  mov   rax, qword [rdi+BOEUF_METADATA_OFF_SIZE]
  add   rax, qword [rsp+0x18]
  add   rax, BOEUF_METADATA_LEN

  ; here, we doubld the size, so that we can avoid malloc for each append
  xor   rdx, rdx
  mov   rbx, 2
  mul   rbx

  ; make sure the requested size is not over MAX_BOEUF_BUF_SIZE
  mov   qword [boeuf_errno], BOEUF_BUF_TOO_LARGE
  cmp   rax, MAX_BOEUF_BUF_SIZE
  jg    .error

  ; realloc chunk of memory
  mov   rdi, [rsp+0x10]
  mov   rsi, rax
  call  realloc

  mov   qword [boeuf_errno], BOEUF_MEMORY_ERROR
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x28], rax

  ; populate metadata of new boeuf buffer
  mov   rbx, qword [BOEUF_MAGIC_VALUE]
  mov   qword [rax+BOEUF_METADATA_OFF_MAGIC_VALUE], rbx

  ; add size of the boeuf buffer
  mov   rbx, qword [rsp+0x20]
  mov   qword [rax+BOEUF_METADATA_OFF_SIZE], rbx

  ; compute new length of the buffer
  mov   rdi, [rsp+0x10]

  ; get current length of old buffer
  mov   rbx, [rdi+BOEUF_METADATA_OFF_LENGTH]

  ; add length of string to append
  add   rbx, qword [rsp+0x18]
  mov   qword [rax+BOEUF_METADATA_OFF_LENGTH], rbx

  ; go after buffer to append second string
  add   rax, BOEUF_METADATA_OFF_DATA
  add   rax, [rdi+BOEUF_METADATA_OFF_LENGTH]
  dec   rax ; go previous null char

  ; move second string
  mov   rdi, rax
  mov   rsi, [rsp+0x8]
  mov   rcx, [rsp+0x18]
  rep   movsb

  ; add null_char
  mov   rax, NULL_CHAR
  stosb

  mov   qword [boeuf_errno], BOEUF_MEMORY_ERROR
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x28]
  add   rax, BOEUF_METADATA_OFF_DATA

  mov   qword [boeuf_errno], BOEUF_NO_ERR
  jmp   .return

.no_grow:
  mov   rdi, [rsp+0x10]
  mov   rax, qword [rdi+BOEUF_METADATA_OFF_LENGTH]

  mov   rcx, qword [rsp+0x18]
  add   qword [rdi+BOEUF_METADATA_OFF_LENGTH], rcx

  mov   rdi, [rsp]
  add   rdi, rax

  mov   rsi, [rsp+0x8]
  rep   movsb

  ; add null char
  mov   rax, NULL_CHAR
  stosb

  mov   qword [boeuf_errno], BOEUF_NO_ERR

  mov   rax, [rsp]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x30
  ret

; appends string to a buffer
; @param  rdi: pointer to the boeuf buffer
; @param  rsi: pointer to the string to be appended
; @return rax: return code
boeuf_append:
  sub   rsp, 0x10

  mov   qword [boeuf_errno], BOEUF_INVALID_ARG
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the boeuf buffer
  ; [rsp+0x8]   -> pointer to the string

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  ; get length of string to be appended
  mov   rdi, [rsp+0x8]
  call  strlen

  mov   qword [boeuf_errno], BOEUF_INVALID_ARG
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  mov   rdx, rax
  call  boeuf_nappend

  mov   qword [boeuf_errno], BOEUF_NO_ERR
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
  mov   rbx, qword [BOEUF_MAGIC_VALUE]

  mov   qword [boeuf_errno], BOEUF_CORRUPTED_BUF
  cmp   qword [rdi+BOEUF_METADATA_OFF_MAGIC_VALUE], rbx
  jne   .error

  call  free

  mov   qword [boeuf_errno], BOEUF_NO_ERR
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; resets the boeuf buffer
; @param  rdi: pointer to the boeuf buffer
; @return rax: return code
boeuf_reset:
  mov   qword [boeuf_errno], BOEUF_INVALID_ARG
  cmp   rdi, 0
  jl    .error

  ; add null char at the beginning of the string
  mov   rax, NULL_CHAR
  stosb

  dec   rdi
  sub   rdi, BOEUF_METADATA_LEN

  ; make sure of the integrity of the boeuf buffer
  mov   rbx, qword [BOEUF_MAGIC_VALUE]
  mov   qword [boeuf_errno], BOEUF_CORRUPTED_BUF
  cmp   qword [rdi+BOEUF_METADATA_OFF_MAGIC_VALUE], rbx
  jne   .error

  ; set length to 0
  mov   qword [rdi+BOEUF_METADATA_OFF_LENGTH], 0

  mov   qword [boeuf_errno], BOEUF_NO_ERR

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
  mov   qword [boeuf_errno], BOEUF_INVALID_ARG
  cmp   rdi, 0
  jl    .error

  sub   rdi, BOEUF_METADATA_LEN

  ; make sure of the integrity of the boeuf buffer
  mov   rbx, qword [BOEUF_MAGIC_VALUE]
  mov   qword [boeuf_errno], BOEUF_CORRUPTED_BUF
  cmp   qword [rdi+BOEUF_METADATA_OFF_MAGIC_VALUE], rbx
  jne   .error

  mov   qword [boeuf_errno], BOEUF_NO_ERR

  mov   rax, qword [rdi+BOEUF_METADATA_OFF_LENGTH]
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

  mov   qword [boeuf_errno], BOEUF_INVALID_ARG
  cmp   rdi, 0
  jl    .error

  cmp   rsi, 0
  jl    .error

  ; *** STACK USAGE *** ;
  ; [rsp]     -> pointer to the boeuf buffer
  ; [rsp+0x8] -> new size of the boeuf buffer

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  sub   rdi, BOEUF_METADATA_LEN

  ; make sure of the integrity of the boeuf buffer
  mov   rbx, qword [BOEUF_MAGIC_VALUE]
  mov   qword [boeuf_errno], BOEUF_CORRUPTED_BUF
  cmp   qword [rdi+BOEUF_METADATA_OFF_MAGIC_VALUE], rbx
  jne   .error

  mov   qword [boeuf_errno], BOEUF_NO_ERR
  cmp   rsi, qword [rdi+BOEUF_METADATA_OFF_LENGTH]
  jge   .return

  mov   qword [rdi+BOEUF_METADATA_OFF_LENGTH], rsi
  add   rdi, BOEUF_METADATA_LEN
  add   rdi, rsi

  mov   rax, NULL_CHAR
  stosb

  mov   qword [boeuf_errno], BOEUF_NO_ERR

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

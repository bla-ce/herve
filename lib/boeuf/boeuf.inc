section .bss

_boeuf_t:
  .magic_value  resq 1
  .length       resq 1
  .capacity     resq 1
  .max_capacity resq 1
  .data         resq 1
_boeuf_t_end:

section .data

_BOEUF_STRUCT_LEN equ _boeuf_t_end - _boeuf_t

_BOEUF_OFF_MAGIC_VALUE  equ 0x0
_BOEUF_OFF_LENGTH       equ _BOEUF_OFF_MAGIC_VALUE + 0x8
_BOEUF_OFF_CAPACITY     equ _BOEUF_OFF_LENGTH + 0x8
_BOEUF_OFF_MAX_CAPACITY equ _BOEUF_OFF_CAPACITY + 0x8
_BOEUF_OFF_DATA         equ _BOEUF_OFF_MAX_CAPACITY + 0x8

_BOEUF_MAGIC_VALUE equ 0xC0FFEE

_BOEUF_DEFAULT_MAX_CAPACITY equ 4096 ; 4kb

section .text
; appends n bytes to the boeuf data which is reallocated if needed
; @param  rdi: pointer to the boeuf struct
; @param  rsi: pointer to the bytes
; @param  rdx: number of bytes to copy
; @return rax: return code
boeuf_nappend:
  sub   rsp, 0x40

  ; STACK USAGE
  ; [rsp]       -> pointer to the boeuf struct
  ; [rsp+0x8]   -> pointer to the bytes
  ; [rsp+0x10]  -> number of bytes to copy
  ; [rsp+0x18]  -> old length of the data
  ; [rsp+0x20]  -> new length of the data
  ; [rsp+0x28]  -> old capacity of the boeuf
  ; [rsp+0x30]  -> new capacity of the boeuf
  ; [rsp+0x38]  -> pointer to the new data

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   qword [rsp+0x38], 0

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .done

  test  rdx, rdx
  jz    .done

  ; make sure that the boeuf is not corrupted
  call  boeuf_is_corrupted
  cmp   rax, FALSE
  jne   .error

  ; get the new length of the data
  mov   rdi, [rsp]
  call  _boeuf_get_len
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax
  add   rax, [rsp+0x10]
  mov   [rsp+0x20], rax

  ; check if new length exceed boeuf max capacity
  mov   rdi, [rsp]
  call  _boeuf_get_max_cap
  cmp   rax, 0
  jl    .error

  cmp   [rsp+0x20], rax
  jge   .error

  ; check against current capacity to check if reallocating is
  ; necessary
  mov   rdi, [rsp]
  call  _boeuf_get_cap
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x28], rax

  cmp   [rsp+0x20], rax
  jg    .realloc

  ; we can just copy the bytes
  mov   rdi, [rsp]
  call  boeuf_get_data
  cmp   rax, 0
  jl    .error

  ; copy the bytes
  mov   rdi, rax
  add   rdi, [rsp+0x18]
  mov   rsi, [rsp+0x8]
  mov   rcx, [rsp+0x10]
  rep   movsb

  ; update length of the data
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x20]
  call  _boeuf_set_len
  cmp   rax, 0
  jl    .error

  jmp   .done

.realloc:
  ; double cap size until it's greater than the new length
  mov   rax, [rsp+0x28]
  inc   rax ; add one as initial cap is 0

.loop:
  shl   rax, 1

  cmp   rax, [rsp+0x20]
  jl    .loop
.loop_end:
  mov   [rsp+0x30], rax

  mov   rdi, [rsp]
  call  boeuf_get_data
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, [rsp+0x30]
  call  realloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x38], rax

  mov   rdi, [rsp]
  mov   rsi, rax
  call  _boeuf_set_data
  cmp   rax, 0
  jl    .error

  ; copy new data
  mov   rdi, [rsp+0x38]
  add   rdi, [rsp+0x18]
  mov   rsi, [rsp+0x8]
  mov   rcx, [rsp+0x10]
  rep   movsb

  ; update new len and new capacity
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x20]
  call  _boeuf_set_len
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x30]
  call  _boeuf_set_cap
  cmp   rax, 0
  jl    .error

.done:
  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rdi, [rsp+0x38]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x40
  ret

; converts the data into a null-terminated string
; user is responsible for having a valid string in data
; user is responsible for freeing the string
; @param  rdi: pointer to the boeuf struct
; @return rax: pointer to the allocated string
boeuf_to_str:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> pointer to the boeuf struct
  ; [rsp+0x8] -> pointer to the data

  mov   [rsp], rdi

  test  rdi, rdi
  jz    .error

  ; make sure that the boeuf is not corrupted
  call  boeuf_is_corrupted
  cmp   rax, FALSE
  jne   .error

  mov   rdi, [rsp]
  call  boeuf_get_data
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; get boeuf length
  mov   rdi, [rsp]
  call  _boeuf_get_len
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  strndup
  cmp   rax, 0
  jl    .error

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; clears boeuf data
; @param  rdi: pointer to the boeuf struct
; @return rax: return code
boeuf_clear:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp]     -> pointer to the boeuf struct

  mov   [rsp], rdi

  test  rdi, rdi
  jz    .error

  ; make sure that the boeuf is not corrupted
  call  boeuf_is_corrupted
  cmp   rax, FALSE
  jne   .error

  mov   rdi, [rsp]
  mov   qword [rdi+_BOEUF_OFF_LENGTH], 0

  ; set the first byte as null
  mov   rax, [rdi+_BOEUF_OFF_DATA]
  mov   byte [rax], NULL_CHAR

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; appends an assumed null terminated string to the boeuf data
; @param  rdi: pointer to the boeuf struct
; @param  rsi: pointer to the null terminated string
; @return rax: return code
boeuf_append_str:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> pointer to the boeuf struct
  ; [rsp+0x8] -> pointer to the null terminated string

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  ; make sure that the boeuf is not corrupted
  call  boeuf_is_corrupted
  cmp   rax, FALSE
  jne   .error

  ; get length of the string
  mov   rdi, [rsp+0x8]
  call  strlen
  cmp   rax, 0
  jle   .error

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  mov   rdx, rax
  call  boeuf_nappend
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; initialises an empty boeuf
; @return rax: pointer to the boeuf struct
boeuf_init:
  mov   rdi, _BOEUF_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   qword [rax+_BOEUF_OFF_MAGIC_VALUE], _BOEUF_MAGIC_VALUE
  mov   qword [rax+_BOEUF_OFF_LENGTH], 0
  mov   qword [rax+_BOEUF_OFF_CAPACITY], 0
  mov   qword [rax+_BOEUF_OFF_MAX_CAPACITY], _BOEUF_DEFAULT_MAX_CAPACITY
  mov   qword [rax+_BOEUF_OFF_DATA], 0

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; frees a boeuf struct and associated resources
; @param  rdi: pointer to the boeuf struct
; @return rax: return code
boeuf_free:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp]   -> pointer to the boeuf struct

  mov   [rsp], rdi

  test  rdi, rdi
  jz    .error

  call  boeuf_is_corrupted
  cmp   rax, FALSE
  jne   .error

  ; free the data
  mov   rdi, [rsp]
  call  boeuf_get_data
  cmp   rax, 0
  jl    .error

  test  rax, rax
  jz    .free_struct

  mov   rdi, rax
  call  free
  cmp   rax, 0
  jl    .error

.free_struct:
  mov   rdi, [rsp]
  call  free
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; checks whether the boeuf struct is corrupted
; @param  rdi: pointer to the boeuf struct
; @return rax: boeuf is corrupted (bool)
boeuf_is_corrupted:
  test  rdi, rdi
  jz    .error

  cmp   qword [rdi+_BOEUF_OFF_MAGIC_VALUE], _BOEUF_MAGIC_VALUE
  jne   .error

  mov   rax, FALSE

  jmp   .return

.error:
  mov   rax, TRUE

.return:
  ret

; sets the data of the boeuf struct
; @param  rdi: pointer to the boeuf struct
; @param  rsi: pointer to the data
; @return rax: return code
_boeuf_set_data:
  test  rdi, rdi
  jz    .error

  mov   [rdi+_BOEUF_OFF_DATA], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the data of the boeuf struct
; @param  rdi: pointer to the boeuf struct
; @return rax: pointer to the data
boeuf_get_data:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp]   -> pointer to the boeuf struct

  mov   [rsp], rdi

  test  rdi, rdi
  jz    .error

  call  boeuf_is_corrupted
  cmp   rax, FALSE
  jne   .error

  mov   rdi, [rsp]
  mov   rax, [rdi+_BOEUF_OFF_DATA]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; sets the length of the boeuf struct
; @param  rdi: pointer to the boeuf struct
; @param  rsi: length
; @return rax: return code
_boeuf_set_len:
  test  rdi, rdi
  jz    .error

  mov   [rdi+_BOEUF_OFF_LENGTH], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the length of the boeuf struct
; @param  rdi: pointer to the boeuf struct
; @return rax: length
_boeuf_get_len:
  test  rdi, rdi
  jz    .error

  mov   rax, [rdi+_BOEUF_OFF_LENGTH]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the capacity of the boeuf struct
; @param  rdi: pointer to the boeuf struct
; @param  rsi: capacity
; @return rax: return code
_boeuf_set_cap:
  test  rdi, rdi
  jz    .error

  mov   [rdi+_BOEUF_OFF_CAPACITY], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the capacity of the boeuf struct
; @param  rdi: pointer to the boeuf struct
; @return rax: capacity
_boeuf_get_cap:
  test  rdi, rdi
  jz    .error

  mov   rax, [rdi+_BOEUF_OFF_CAPACITY]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the max capacity of the boeuf struct
; @param  rdi: pointer to the boeuf struct
; @return rax: max capacity
_boeuf_get_max_cap:
  test  rdi, rdi
  jz    .error

  mov   rax, [rdi+_BOEUF_OFF_MAX_CAPACITY]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

boeuf_create:
boeuf_append:
boeuf_ncreate:

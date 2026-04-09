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
  mov   qword [rax+_BOEUF_OFF_MAX_CAPACITY], 0
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

boeuf_append:
boeuf_nappend:
boeuf_create:
boeuf_ncreate:

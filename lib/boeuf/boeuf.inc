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

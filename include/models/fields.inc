section .bss

field_t:
  _field_name  resq 1  ; pointer to the name of the field
  _field_type  resq 1  ; field type
  _field_arg   resq 1  ; specific argument for the field
  _field_next  resq 1  ; next field
field_t_end:

section .data

FIELD_NAME_MAX_LEN  equ 0xFF

FIELD_T_LEN equ field_t_end - field_t

; offsets
FIELD_OFF_NAME  equ 0
FIELD_OFF_TYPE  equ FIELD_OFF_NAME + 0x8
FIELD_OFF_ARG   equ FIELD_OFF_TYPE + 0x8
FIELD_OFF_NEXT  equ FIELD_OFF_ARG + 0x8

; field types
FIELD_TYPE_INTEGER  equ 0
FIELD_TYPE_STRING   equ 1
FIELD_TYPE_BOOL     equ 2
FIELD_TYPE_FLOAT    equ 3

id_field_name db "id", NULL_CHAR

section .text
; creates a field
; @param  rdi: pointer to the name of the field
; @param  rsi: field type
; @param  rdx: arg
; @return rax: pointer to the field struct
field_create:
  sub   rsp, 0x20

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the name of the field
  ; [rsp+0x8]   -> field type
  ; [rsp+0x10]  -> arg
  ; [rsp+0x18]  -> pointer to the field struct

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   qword [rsp+0x18], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  cmp   rsi, 2
  jg    .error

  mov   rdi, FIELD_T_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  mov   rdi, [rsp+0x18]
  mov   rsi, [rsp]
  call  field_set_name
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x18]
  mov   rsi, [rsp+0x8]
  call  field_set_type
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x18]
  mov   rsi, [rsp+0x10]
  call  field_set_arg
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x18]

  jmp   .return

.error:
  mov   rdi, [rsp+0x18]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret

; sets the type of the field
; @param  rdi: pointer to the field struct
; @param  rsi: pointer to the type of the field
; @return rax: return code
field_set_type:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   [rdi+FIELD_OFF_TYPE], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the type of the field
; @param  rdi: pointer to the field struct
; @return rax: pointer to the type of the field
field_get_type:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+FIELD_OFF_TYPE]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets a specific argument for the field
; @param  rdi: pointer to the field struct
; @param  rsi: specific argument
; @return rax: return code
field_set_arg:
  cmp   rdi, 0
  jle   .error

  mov   [rdi+FIELD_OFF_ARG], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the specific argument of the field
; @param  rdi: pointer to the field struct
; @return rax: specific argument
field_get_arg:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+FIELD_OFF_ARG]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the name of the field
; @param  rdi: pointer to the field struct
; @param  rsi: pointer to the name of the field
; @return rax: return code
field_set_name:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   [rdi+FIELD_OFF_NAME], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the name of the field
; @param  rdi: pointer to the field struct
; @return rax: pointer to the name of the field
field_get_name:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+FIELD_OFF_NAME]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; destroys the field and associates resources
; @param  rdi: pointer to the field struct
; @return rax: return code
field_free:
  sub   rsp, 0x8

  ; *** STACK USAGE *** ;
  ; [rsp]   -> pointer to the field struct

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

  ; free struct
  mov   rdi, [rsp]
  call  free
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

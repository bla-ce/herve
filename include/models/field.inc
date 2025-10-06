section .bss

field_t:
  field_t_name  resq 1
  field_t_type  resq 1
  field_t_arg   resq 1
  field_t_next  resq 1
field_t_end:

section .data

FIELD_T_LEN equ field_t_end - field_t

; field offsets
FIELD_OFF_NAME  equ 0
FIELD_OFF_TYPE  equ FIELD_OFF_NAME + 0x8
FIELD_OFF_ARG   equ FIELD_OFF_TYPE + 0x8
FIELD_OFF_NEXT  equ FIELD_OFF_ARG + 0x8

; field types
FIELD_OFF_BOOL    equ 0
FIELD_OFF_STRING  equ 1
FIELD_OFF_INT     equ 2

STRING_DEFAULT_LEN equ 255

field_id_name db "id", NULL_CHAR

section .text
; inserts a boolean field to a model
; @param  rdi: pointer to the model struct
; @param  rsi: pointer to the name of the field
; @return rax: pointer to the field struct
model_insert_bool:
  sub   rsp, 0x18

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the model struct
  ; [rsp+0x8]   -> pointer to the name of the field
  ; [rsp+0x10]  -> pointer to the field struct

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x10], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; malloc field struct
  mov   rdi, FIELD_T_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; populate field struct
  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp+0x8]
  call  field_set_name
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x10]
  mov   rsi, FIELD_OFF_BOOL
  call  field_set_type
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x10]
  xor   rsi, rsi
  call  field_set_next
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x10]

  jmp   .return

.error:
  mov   rdi, [rsp+0x10]
  test  rdi, rdi
  jz    .no_free
  
  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; inserts an integer field to a model
; @param  rdi: pointer to the model struct
; @param  rsi: pointer to the name of the field
; @return rax: pointer to the field struct
model_insert_int:
  sub   rsp, 0x18

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the model struct
  ; [rsp+0x8]   -> pointer to the name of the field
  ; [rsp+0x10]  -> pointer to the field struct

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x10], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; malloc field struct
  mov   rdi, FIELD_T_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; populate field struct
  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp+0x8]
  call  field_set_name
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x10]
  mov   rsi, FIELD_OFF_INT
  call  field_set_type
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x10]
  xor   rsi, rsi
  call  field_set_next
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x10]

  jmp   .return

.error:
  mov   rdi, [rsp+0x10]
  test  rdi, rdi
  jz    .no_free
  
  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; inserts a string field to a model
; @param  rdi: pointer to the model struct
; @param  rsi: pointer to the name of the field
; @param  rdx: max length of the string
; @return rax: pointer to the field struct
model_insert_string:
  sub   rsp, 0x20

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the model struct
  ; [rsp+0x8]   -> pointer to the name of the field
  ; [rsp+0x10]  -> max length of the string
  ; [rsp+0x18]  -> pointer to the field struct

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   qword [rsp+0x18], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov     rbx, STRING_DEFAULT_LEN
  cmp     rdx, 0
  cmovle  rdx, rbx

  ; malloc field struct
  mov   rdi, FIELD_T_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; populate field struct
  mov   rdi, [rsp+0x18]
  mov   rsi, [rsp+0x8]
  call  field_set_name
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x18]
  mov   rsi, FIELD_OFF_STRING
  call  field_set_type
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x18]
  mov   rsi, [rsp+0x10]
  call  field_set_arg
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x18]
  xor   rsi, rsi
  call  field_set_next
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

; sets the name of the field struct
; @param  rdi: pointer to the field struct
; @param  rsi: pointer to the name
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

; returns the name of the field struct
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

; sets the type of the field struct
; @param  rdi: pointer to the field struct
; @param  rsi: pointer to the type of the field
; @return rax: return code
field_set_type:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   [rdi+FIELD_OFF_TYPE], rsi

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the type of the field struct
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

; sets the specific argument of the field struct
; @param  rdi: pointer to the field struct
; @param  rsi: argument of the field
; @return rax: return code
field_set_arg:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   [rdi+FIELD_OFF_ARG], rsi

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the arg of the field struct
; @param  rdi: pointer to the field struct
; @return rax: argument of the field
field_get_arg:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+FIELD_OFF_ARG]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the pointer of the next field struct
; @param  rdi: pointer to the field struct
; @return rax: pointer to the next field struct
field_get_next:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+FIELD_OFF_NEXT]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the next field struct
; @param  rdi: pointer to the field struct
; @param  rsi: pointer to the next field struct
; @return rax: return code
field_set_next:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   [rdi+FIELD_OFF_NEXT], rsi

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret


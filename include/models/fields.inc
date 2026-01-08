section .bss

field_t:
  _field_name     resq 1     ; pointer to the name of the field
  _field_type     resq 1     ; field type
  _field_arg      resq 1     ; specific argument for the field
  _field_required resq 1  ; whether the field is required for POST
  _field_next     resq 1     ; next field
field_t_end:

section .data

FIELD_NAME_MAX_LEN  equ 0xFF

FIELD_T_LEN equ field_t_end - field_t

; offsets
FIELD_OFF_NAME      equ 0
FIELD_OFF_TYPE      equ FIELD_OFF_NAME + 0x8
FIELD_OFF_ARG       equ FIELD_OFF_TYPE + 0x8
FIELD_OFF_REQUIRED  equ FIELD_OFF_ARG + 0x8
FIELD_OFF_NEXT      equ FIELD_OFF_REQUIRED + 0x8

; field types
FIELD_TYPE_INTEGER  equ 0
FIELD_TYPE_STRING   equ 1
FIELD_TYPE_BOOL     equ 2
FIELD_TYPE_FLOAT    equ 3

id_field_name         db "id", NULL_CHAR
created_at_field_name db "created_at", NULL_CHAR
updated_at_field_name db "updated_at", NULL_CHAR

section .text
; creates a field
; @param  rdi: pointer to the name of the field
; @param  rsi: field type
; @param  rdx: arg
; @param  rcx: whether the field is required
; @return rax: pointer to the field struct
field_create:
  sub   rsp, 0x28

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the name of the field
  ; [rsp+0x8]   -> field type
  ; [rsp+0x10]  -> arg
  ; [rsp+0x18]  -> whether the field is required
  ; [rsp+0x20]  -> pointer to the field struct

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   [rsp+0x18], rcx
  mov   qword [rsp+0x20], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  cmp   rsi, FIELD_TYPE_FLOAT
  jg    .error

  mov   rdi, FIELD_T_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  mov   rdi, [rsp+0x20]
  mov   rsi, [rsp]
  call  field_set_name
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x20]
  mov   rsi, [rsp+0x8]
  call  field_set_type
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x20]
  mov   rsi, [rsp+0x10]
  call  field_set_arg
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x20]
  mov   rsi, [rsp+0x18]
  call  field_set_required
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x20]

  jmp   .return

.error:
  mov   rdi, [rsp+0x20]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x28
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

; sets the required flag of the field
; @param  rdi: pointer to the field struct
; @param  rsi: whether the field is required
; @return rax: return code
field_set_required:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   [rdi+FIELD_OFF_REQUIRED], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the required flag of the field
; @param  rdi: pointer to the field struct
; @return rax: whether the field is required
field_get_required:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+FIELD_OFF_REQUIRED]

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

; sets the next pointer of the field
; @param  rdi: pointer to the field struct
; @param  rsi: pointer to the next field struct
; @return rax: return code
field_set_next:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   [rdi+FIELD_OFF_NEXT], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns next field struct
; @param  rdi: pointer to the field struct
; @return rax: pointer to next field struct
field_get_next:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+FIELD_OFF_NEXT]

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

; converts the value of the field depending on its type
; @param  rdi: pointer to the field struct
; @param  rsi: value
; @return rax: converted value
field_convert_value:
  sub   rsp, 0x10

  ; *** STACK USAGE *** ;
  ; [rsp]     -> pointer to the field struct
  ; [rsp+0x8] -> value

  cmp   rdi, 0
  jle   .error

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  ; get type of the field
  mov   rdi, [rsp]
  call  field_get_type
  cmp   rax, 0
  jl    .error

  cmp   rax, FIELD_TYPE_INTEGER
  je    .to_integer

  cmp   rax, FIELD_TYPE_STRING
  je    .to_string

  cmp   rax, FIELD_TYPE_BOOL
  je    .to_bool

  ; unknown / not implemented type
  jmp   .error

.to_integer:
  mov   rdi, [rsp+0x8]
  call  stoi

  jmp   .return

.to_bool:
  mov   rdi, [rsp+0x8]
  call  string_to_bool
  cmp   rax, 0
  jl    .error

  jmp   .return

.to_string:
  mov   rax, [rsp+0x8]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

section .bss

_json_entry_t:
  .data   resq 0
  .child  resq 0
  .key    resq 0
  .type   resb 0
_json_entry_t_end:

_json_t:
  .data dq 0
  ; TODO: check what can be added here
_json_t_end:

section .data

JSON_STRUCT_LEN equ _json_t_end - _json_t

JSON_ENTRY_OFF_DATA   equ 0x0
JSON_ENTRY_OFF_CHILD  equ JSON_ENTRY_OFF_DATA + 0x8
JSON_ENTRY_OFF_KEY    equ JSON_ENTRY_OFF_CHILD + 0x8
JSON_ENTRY_OFF_TYPE   equ JSON_ENTRY_OFF_KEY + 0x8

JSON_ENTRY_STRUCT_LEN equ _json_entry_t_end - _json_entry_t

JSON_OFF_DATA equ 0x0

; types
JSON_TYPE_BOOL   equ 0x0
JSON_TYPE_STRING equ 0x1
JSON_TYPE_NUMBER equ 0x2
JSON_TYPE_DOUBLE equ 0x3
JSON_TYPE_ARRAY  equ 0x4
JSON_TYPE_OBJECT equ 0x5
JSON_TYPE_NULL   equ 0x6

JSON_FALSE equ 0x0
JSON_TRUE  equ 0x1

section .text

; creates an json struct
; @return rax: pointer to the json struct
json_create:
  mov   rdi, JSON_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   qword [rax+JSON_OFF_DATA], 0
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; creates a json entry with data and type
; key and child is defined later with json_add_item_to_... methods
; @param  rdi: data
; @param  rsi: type
; @return rax: pointer to the json entry
_json_create_entry:
  sub   rsp, 0x18

  ; [rsp]       -> data of the entry
  ; [rsp+0x8]   -> type of the entry
  ; [rsp+0x10]  -> pointer to the json entry

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x10], 0

  mov   rdi, rsi
  call  _json_validate_type
  cmp   rax, 0
  jl    .error

  mov   rdi, JSON_ENTRY_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  mov   rdi, rax
  mov   rsi, [rsp]
  call  _json_entry_set_data
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp+0x8]
  call  _json_entry_set_type
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x10]
  xor   rsi, rsi
  call  _json_entry_set_child
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x10]
  xor   rsi, rsi
  call  _json_entry_set_key
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x10]

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

; creates a json entry with string type
; @param  rdi: pointer to the string
; @return rax: pointer to the json entry
json_create_string:
  sub   rsp, 0x8

  test  rdi, rdi
  jz    .error

  ; duplicate string
  call  strdup
  cmp   rax, 0
  jl    .error

  mov   [rsp], rax

  mov   rdi, rax
  mov   rsi, JSON_TYPE_STRING
  call  _json_create_entry
  cmp   rax, 0
  jl    .error

  jmp   .return

.error:
  mov   rdi, [rsp]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; creates a json entry with boolean type
; @param  rdi: bool value (JSON_TRUE, JSON_FALSE)
; @return rax: pointer to the json entry
json_create_bool:
  cmp   rdi, JSON_FALSE
  je    .init_stack

  cmp   rdi, JSON_TRUE
  je    .init_stack

  ; invalid value
  jmp   .error

.init_stack:
  mov   rsi, JSON_TYPE_BOOL
  call  _json_create_entry
  cmp   rax, 0
  jl    .error

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; validates the type
; @param  rdi: type value
; @return rax: type is valid
_json_validate_type:
  cmp   rdi, JSON_TYPE_BOOL
  jl    .not_valid

  cmp   rdi, JSON_TYPE_NULL
  jg    .not_valid

.valid:
  mov   rax, TRUE
  ret

.not_valid:
  mov   rax, FALSE
  ret

; sets the data of the json entry
; @param  rdi: pointer to the json entry
; @param  rsi: data
; @return rax: return code
_json_entry_set_data:
  test  rdi, rdi
  jz    .error

  mov   [rdi+JSON_ENTRY_OFF_DATA], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the key of the json entry
; @param  rdi: pointer to the json entry
; @param  rsi: pointer to the key
; @return rax: return code
_json_entry_set_key:
  test  rdi, rdi
  jz    .error

  mov   [rdi+JSON_ENTRY_OFF_KEY], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the type of the json entry
; @param  rdi: pointer to the json entry
; @param  rsi: type
; @return rax: return code
_json_entry_set_type:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> pointer to the json entry
  ; [rsp+0x8] -> type of the value

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  test  rdi, rdi
  jz    .error

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]

  mov   [rdi+JSON_ENTRY_OFF_TYPE], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the child of the json entry
; @param  rdi: pointer to the json entry
; @param  rsi: pointer to the child
; @return rax: return code
_json_entry_set_child:
  test  rdi, rdi
  jz    .error

  mov   [rdi+JSON_ENTRY_OFF_CHILD], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

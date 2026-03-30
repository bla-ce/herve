section .bss

_json_entry_t:
  .data   resq 0
  .child  resq 0
  .key    resq 0
  .type   resb 0
_json_entry_t_end:

section .data

JSON_ENTRY_OFF_DATA   equ 0x0
JSON_ENTRY_OFF_CHILD  equ JSON_ENTRY_OFF_DATA + 0x8
JSON_ENTRY_OFF_KEY    equ JSON_ENTRY_OFF_CHILD + 0x8
JSON_ENTRY_OFF_TYPE   equ JSON_ENTRY_OFF_KEY + 0x8

JSON_ENTRY_STRUCT_LEN equ _json_entry_t_end - _json_entry_t

; types
JSON_TYPE_BOOL   equ 0x0
JSON_TYPE_STRING equ 0x1
JSON_TYPE_NUMBER equ 0x2
JSON_TYPE_NULL   equ 0x3
JSON_TYPE_ARRAY  equ 0x4
JSON_TYPE_OBJECT equ 0x5

JSON_NULL   equ 0x0
JSON_FALSE  equ 0x0
JSON_TRUE   equ 0x1

JSON_NUMBER_MAX equ 10000000000
JSON_NUMBER_MIN equ -10000000000

section .text
; adds a new entry to a json object
; @param  rdi: pointer to the json structure
; @param  rsi: pointer to the entry to add
; @param  rdx: key of the entry
; @return rax: return code
json_add_entry_to_object:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> pointer to the json structure
  ; [rsp+0x8]   -> pointer to the entry to add
  ; [rsp+0x10]  -> pointer to the key of the entry

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  test  rdx, rdx
  jz    .error

  ; set entry key
  mov   rdi, rsi
  mov   rsi, rdx
  call  _json_entry_set_key
  cmp   rax, 0
  jl    .error

  ; get children
  mov   rax, [rsp]
  lea   rdi, [rax+JSON_ENTRY_OFF_CHILD]
  mov   rsi, [rsp+0x8]
  call  linked_list_insert_at_end
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; adds a new entry to a json array
; @param  rdi: pointer to the json structure
; @param  rsi: pointer to the entry to add
; @return rax: return code
json_add_entry_to_array:
  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  ; get children
  lea   rdi, [rdi+JSON_ENTRY_OFF_CHILD]
  mov   rsi, [rsp+0x8]
  call  linked_list_insert_at_end
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rax, SUCCESS_CODE

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

; creates a json object
; @return rax: pointer to the json entry
json_create_object:
  mov   rdi, NO_ARG
  mov   rsi, JSON_TYPE_OBJECT
  call  _json_create_entry
  cmp   rax, 0
  jl    .error

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; creates a json array
; @return rax: pointer to the json entry
json_create_array:
  mov   rdi, NO_ARG
  mov   rsi, JSON_TYPE_ARRAY
  call  _json_create_entry
  cmp   rax, 0
  jl    .error

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; creates a json entry with null value
; @return rax: pointer to the json entry
json_create_null:
  mov   rdi, JSON_NULL
  mov   rsi, JSON_TYPE_NULL
  call  _json_create_entry
  cmp   rax, 0
  jl    .error

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; creates a json entry with number type
; @param  rdi: number value
; @return rax: pointer to the json entry
json_create_number:
  mov   rax, JSON_NUMBER_MAX
  cmp   rdi, rax
  jg    .error

  mov   rax, JSON_NUMBER_MIN
  cmp   rdi, rax
  jl    .error

  mov   rsi, JSON_TYPE_NUMBER
  call  _json_create_entry
  cmp   rax, 0
  jl    .error

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; creates a json entry with a string
; @param  rdi: pointer to the string
; @return rax: pointer to the json entry
json_create_string:
  test  rdi, rdi
  jz    .error

  mov   rsi, JSON_TYPE_STRING
  call  _json_create_entry
  cmp   rax, 0
  jl    .error

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
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
  cmp   dil, JSON_TYPE_BOOL
  jl    .not_valid

  cmp   dil, JSON_TYPE_NULL
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
  test  rdi, rdi
  jz    .error

  mov   byte [rdi+JSON_ENTRY_OFF_TYPE], sil

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

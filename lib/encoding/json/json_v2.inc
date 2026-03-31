section .bss

_json_t:
  .data   resq 1
  .child  resq 1
  .key    resq 1
  .type   resb 1
_json_t_end:

section .data

JSON_OFF_DATA   equ 0x0
JSON_OFF_CHILD  equ JSON_OFF_DATA + 0x8
JSON_OFF_KEY    equ JSON_OFF_CHILD + 0x8
JSON_OFF_TYPE   equ JSON_OFF_KEY + 0x8

JSON_STRUCT_LEN equ _json_t_end - _json_t

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

; json string
JSON_BEGIN      db "{", NULL_CHAR
JSON_END        db "}", NULL_CHAR

JSON_ARRAY_BEGIN  db "[", NULL_CHAR
JSON_ARRAY_END    db "]", NULL_CHAR

JSON_STR_QUOTES db DOUBLE_QUOTES, NULL_CHAR
JSON_KV_DELIM   db ":", NULL_CHAR
JSON_KEYS_DELIM db ",", NULL_CHAR

JSON_BOOL_FALSE db "false", NULL_CHAR
JSON_BOOL_TRUE  db "true", NULL_CHAR

JSON_NULL db "null", NULL_CHAR

JSON_FLOAT_DOT db ".", NULL_CHAR

section .text
; stringifies a json entry
; the user is responsible for freeing the string
; @param  rdi: pointer to the json struct
; @return rax: pointer to the allocated string
json_to_string:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> pointer to the json struct
  ; [rsp+0x8] -> pointer to the boeuf buffer

  mov   [rsp], rdi
  mov   qword [rsp+0x8], 0

  test  rdi, rdi
  jz    .error

  ; stringify value


  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rdi, [rsp+0x8]
  test  rdi, rdi
  jz    .no_free

  call  boeuf_free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; adds a new entry to a json object
; @param  rdi: pointer to the json structure
; @param  rsi: pointer to the entry to add
; @param  rdx: key of the entry
; @return rax: return code
json_add_entry_to_object:
  sub   rsp, 0x20

  ; STACK USAGE
  ; [rsp]       -> pointer to the json structure
  ; [rsp+0x8]   -> pointer to the entry to add
  ; [rsp+0x10]  -> pointer to the key of the entry
  ; [rsp+0x18]  -> pointer to the dup key

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   qword [rsp+0x18], 0

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  test  rdx, rdx
  jz    .error

  ; duplicate the key
  mov   rdi, rdx
  call  strdup
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; set entry key
  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  _json_set_key
  cmp   rax, 0
  jl    .error

  ; get children
  mov   rax, [rsp]
  lea   rdi, [rax+JSON_OFF_CHILD]
  mov   rsi, [rsp+0x8]
  call  linked_list_insert_at_end
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rax, SUCCESS_CODE

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

; recursively frees the json struct and all nested entries
; @param  rdi: pointer to the the parent json entry
; @return rax: return code
json_free:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp]   -> pointer to the parent json entry

  mov   [rsp], rdi

  test  rdi, rdi
  jz    .return

  ; frees children; oohhhh recursion, getting fancy here
  ; segfault expected within the next 3 commits
  lea   rdi, [rdi+JSON_OFF_CHILD]
  cmp   qword [rdi], 0
  je    .free_entry

  mov   rsi, json_free
  call  linked_list_free
  cmp   rax, 0
  jl    .error

.free_entry:
  mov   rdi, [rsp]
  call  _json_entry_free
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; frees a json entry
; @param  rdi: pointer to the json entry
; @return rax: return code
_json_entry_free:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp]   -> pointer to the json entry

  mov   [rsp], rdi

  test  rdi, rdi
  jz    .error

  ; if the entry is of type string, free the value
  call  _json_get_type
  cmp   rax, 0
  jl    .error

  cmp   al, JSON_TYPE_STRING
  jne   .free_key

  mov   rdi, [rsp]
  call  _json_get_data
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  free
  cmp   rax, 0
  jl    .error

.free_key:
  ; free the key
  mov   rdi, [rsp]
  call  _json_get_key
  cmp   rax, 0
  jl    .error
  je    .free_entry

  mov   rdi, rax
  call  free
  cmp   rax, 0
  jl    .error

.free_entry:
  ; free the entry
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
  lea   rdi, [rdi+JSON_OFF_CHILD]
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

  mov   rdi, JSON_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  mov   rdi, rax
  mov   rsi, [rsp]
  call  _json_set_data
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp+0x8]
  call  _json_set_type
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x10]
  xor   rsi, rsi
  call  _json_set_child
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x10]
  xor   rsi, rsi
  call  _json_set_key
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

  ; duplicate the string
  call  strdup
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
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
_json_set_data:
  test  rdi, rdi
  jz    .error

  mov   [rdi+JSON_OFF_DATA], rsi

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
_json_set_key:
  test  rdi, rdi
  jz    .error

  mov   [rdi+JSON_OFF_KEY], rsi

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
_json_set_type:
  test  rdi, rdi
  jz    .error

  mov   byte [rdi+JSON_OFF_TYPE], sil

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
_json_set_child:
  test  rdi, rdi
  jz    .error

  mov   [rdi+JSON_OFF_CHILD], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the child of the json entry
; @param  rdi: pointer to the json entry
; @return rax: child of the entry
_json_get_child:
  test  rdi, rdi
  jz    .error

  mov   rax, [rdi+JSON_OFF_CHILD]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the key of the json entry
; @param  rdi: pointer to the json entry
; @return rax: pointer to the key of the entry
_json_get_key:
  test  rdi, rdi
  jz    .error

  mov   rax, [rdi+JSON_OFF_KEY]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the type of the json entry
; @param  rdi: pointer to the json entry
; @return rax: type of the entry
_json_get_type:
  test  rdi, rdi
  jz    .error

  mov   al, byte [rdi+JSON_OFF_TYPE]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the data of the json entry
; @param  rdi: pointer to the json entry
; @return rax: data of the entry
_json_get_data:
  test  rdi, rdi
  jz    .error

  mov   rax, [rdi+JSON_OFF_DATA]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

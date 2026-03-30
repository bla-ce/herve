section .bss

_json_entry_t:
  .data   resq 0
  .child  resq 0
  .key    resq 0
  .type   resb 0
  .level  resb 0
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
JSON_ENTRY_OFF_LEVEL  equ JSON_ENTRY_OFF_TYPE + 0x1

JSON_ENTRY_STRUCT_LEN equ _json_entry_t_end - _json_entry_t

JSON_OFF_DATA equ 0x0

; types
JSON_BOOL   equ 0x0
JSON_STRING equ 0x1
JSON_NUMBER equ 0x2
JSON_DOUBLE equ 0x3
JSON_ARRAY  equ 0x4
JSON_OBJECT equ 0x5
JSON_NULL   equ 0x6

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

; adds a boolean entry to the json
; @param  rdi: bool value (JSON_TRUE, JSON_FALSE)
; @return rax: pointer to the json entry
json_create_bool:

.return:
  ret

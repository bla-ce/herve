section .bss

ht_entry_t:
  .ht_entry_key   resq 1
  .ht_entry_value resq 1
  .ht_entry_next  resq 1
ht_entry_t_end:

section .data

HT_ENTRY_STRUCT_LEN equ ht_entry_t_end - ht_entry_t

HT_ENTRY_OFF_KEY    equ 0
HT_ENTRY_OFF_VALUE  equ HT_ENTRY_OFF_KEY + 0x8
HT_ENTRY_OFF_NEXT   equ HT_ENTRY_OFF_VALUE + 0x8

HT_ENTRY_KEY_MAX_LENGTH   equ 0xFF

section .text

; frees the hash table entry and related resources
; @param  rdi: pointer to the entry
; @return rax: return code
ht_entry_free:
  sub   rsp, 0x8

  ; *** STACK USAGE *** ;
  ; [rsp]   -> pointer to the entry

  cmp   rdi, 0
  jle   .error

  mov   [rsp], rdi

  ; free the key
  mov   rdi, [rsp]
  call  ht_entry_get_key
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  free
  cmp   rax, 0
  jl    .error

  ; free the value
  mov   rdi, [rsp]
  call  ht_entry_get_value
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  free
  cmp   rax, 0
  jl    .error

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

; creates a ht entry
; @param  rdi: pointer to the key
; @param  rdi: pointer to the value
; @return rax: pointer to the entry struct
ht_entry_create:
  sub   rsp, 0x28

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the key
  ; [rsp+0x8]   -> pointer to the value
  ; [rsp+0x10]  -> pointer to the dup key
  ; [rsp+0x18]  -> pointer to the dup value
  ; [rsp+0x20]  -> pointer to the entry struct

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x10], 0
  mov   qword [rsp+0x18], 0
  mov   qword [rsp+0x20], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; duplicate the key
  mov   rdi, [rsp]
  call  strdup
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; duplicate the value
  mov   rdi, [rsp+0x8]
  call  strdup
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; malloc entry
  mov   rdi, HT_ENTRY_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  ; populate entry
  mov   rdi, [rsp+0x20]
  mov   rsi, [rsp+0x10]
  call  ht_entry_set_key
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x20]
  mov   rsi, [rsp+0x18]
  call  ht_entry_set_value
  cmp   rax, 0
  jl    .error

  ; set next as 0
  mov   rdi, [rsp+0x20]
  xor   rsi, rsi
  call  ht_entry_set_next
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x20]

  jmp   .return

.error:
  mov   rdi, [rsp+0x20]
  test  rdi, rdi
  jz    .free_value

  call  free

.free_value:
  mov   rdi, [rsp+0x18]
  test  rdi, rdi
  jz    .free_key

  call  free

.free_key:
  mov   rdi, [rsp+0x10]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x28
  ret

; sets the next entry of a ht entry
; @param  rdi: pointer to the ht entry
; @param  rsi: pointer to next
; @return rax: return code
ht_entry_set_next:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   [rdi+HT_ENTRY_OFF_NEXT], rsi

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the next entry of a ht entry
; @param  rdi: pointer to the ht entry
; @return rax: pointer to next
ht_entry_get_next:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+HT_ENTRY_OFF_NEXT]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the key of a ht entry
; @param  rdi: pointer to the ht entry
; @return rax: pointer to the key
ht_entry_get_key:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+HT_ENTRY_OFF_KEY]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the key of a ht entry
; @param  rdi: pointer to the ht entry
; @param  rsi: pointer to the key
; @return rax: return code
ht_entry_set_key:
  cmp   rdi, 0
  jl    .error

  mov   [rdi+HT_ENTRY_OFF_KEY], rsi

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the value of a ht entry
; @param  rdi: pointer to the ht entry
; @return rax: pointer to the value
ht_entry_get_value:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+HT_ENTRY_OFF_VALUE]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the value of a ht entry
; @param  rdi: pointer to the ht entry
; @param  rsi: pointer to the value
; @return rax: return code
ht_entry_set_value:
  cmp   rdi, 0
  jl    .error

  mov   [rdi+HT_ENTRY_OFF_VALUE], rsi

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

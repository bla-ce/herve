section .text

; adds an entry to a linked list
; @param  rdi: linked list
; @param  rsi: address of the entry to add
; @param  rdx: next offset value
; @return rax: return code
linked_list_add_entry:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> linked list
  ; [rsp+0x8]   -> address of the entry to add
  ; [rsp+0x10]  -> next offset value

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  cmp   rdx, 0
  jl    .error

  ; check if linked list is empty
  mov   rdi, [rsp]
  cmp   qword [rdi], 0
  je    .add

  ; load first entry
  mov   rsi, [rdi]
  mov   rdi, rsi

.loop:
  ; rdi points to the address of current route
  ; go to next offset
  add   rdi, [rsp+0x10]
  cmp   qword [rdi], 0
  je    .add

  ; rdi contains the address of the entry
  mov   rsi, [rdi]
  mov   rdi, rsi

  jmp   .loop

.add:
  mov   rsi, [rsp+0x8]
  mov   [rdi], rsi

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; removes an entry from a linked list
; @param  rdi: linked list
; @param  rsi: address of the entry to remove
; @param  rdx: next offset value
; @return rax: return code
linked_list_remove_entry:
  ret

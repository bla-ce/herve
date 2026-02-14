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
  ; rdi points to the address of current entry
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
  sub   rsp, 0x20

  ; STACK USAGE
  ; [rsp]       -> linked list
  ; [rsp+0x8]   -> address of the entry to add
  ; [rsp+0x10]  -> next offset value
  ; [rsp+0x18]  -> prev value

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   [rsp+0x18], rdi

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  cmp   rdx, 0
  jl    .error

  ; check if linked list is empty
  mov   rdi, [rsp]
  cmp   qword [rdi], 0
  je    .error

  cmp   qword [rdi], rsi
  je    .remove_head

  ; load first entry
  mov   rsi, [rdi]
  mov   rdi, rsi

.loop:
  ; rdi points to the address of the current entry
  cmp   rdi, [rsp+0x8]
  je    .remove

  mov   [rsp+0x18], rdi

  ; go to next
  add   rdi, [rsp+0x10]

  ; rdi contains the address of the entry
  mov   rsi, [rdi]
  mov   rdi, rsi

  cmp   rdi, 0
  je    .error

  jmp   .loop

.remove:
  ; rdi contains the address of the entry to remove
  ; get current next
  add   rdi, [rsp+0x10]
  mov   rax, [rdi]

  ; get prev
  mov   rsi, [rsp+0x18]

  ; change prev next
  add   rsi, [rsp+0x10]
  mov   [rsi], rax

  mov   rax, SUCCESS_CODE

  jmp   .return

.remove_head:
  ; get head
  mov   rdi, [rsp]

  mov   rax, [rsp+0x10]

  ; get head next
  mov   rbx, [rdi]
  mov   rsi, [rbx+rax]

  ; replace head
  mov   [rdi], rsi

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret

; frees all entries from the linked list with a specific free
; function. Users need to make sure that each entry has been
; previously allocated with malloc
; @param  rdi: pointer to the linked list
; @param  rsi: free function for the entries
; @param  rdx: next offset value
; @return rax: return code
linked_list_free:
  sub   rsp, 0x28

  ; STACK USAGE
  ; [rsp]       -> pointer to the linked list
  ; [rsp+0x8]   -> free function for the entries
  ; [rsp+0x10]  -> next offset value
  ; [rsp+0x18]  -> pointer to curr entry
  ; [rsp+0x20]  -> pointer to next entry

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
  je    .loop_end

  ; load first entry
  mov   rsi, [rdi]
  mov   [rsp+0x18], rsi

.loop:
  ; get route next
  mov   rdi, [rsp+0x18]
  mov   rdx, [rsp+0x10]
  mov   rax, [rdi+rdx]

  mov   [rsp+0x20], rax

  ; free the entry with custom free
  mov   rdi, [rsp+0x18]
  mov   rax, [rsp+0x8]
  call  rax
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x20]
  test  rdi, rdi
  jz    .loop_end

  mov   [rsp+0x18], rdi

  jmp   .loop
.loop_end:
  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x28
  ret

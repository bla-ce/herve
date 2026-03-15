section .bss

_node_t:
  .data resq 1
  .next resq 1
_node_t_end:

section .data

NODE_STRUCT_LEN equ _node_t_end - _node_t

NODE_OFF_DATA equ 0x0
NODE_OFF_NEXT equ NODE_OFF_DATA + 0x8

_linked_list_errno  db 0

section .text

; creates a new node to be later added in the linked list
; @param  rdi: pointer to the data
; @return rax: pointer to the node
_linked_list_new_node:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> pointer to the data
  ; [rsp+0x8] -> pointer to the node

  mov   [rsp], rdi
  mov   qword [rsp+0x8], 0

  mov   rdi, NODE_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp]
  call  _linked_list_set_node_data
  cmp   byte [_linked_list_errno], TRUE
  je    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, NO_ARG
  call  _linked_list_set_node_next
  cmp   byte [_linked_list_errno], TRUE
  je    .error

  mov   rax, [rsp+0x8]

  jmp   .return

.error:
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; adds a node to the beginning of the linked list
; @param  rdi: pointer to the pointer to the head
; @param  rsi: pointer to the data
; @return rax: return code
linked_list_insert_at_first:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> pointer to the pointer to the head
  ; [rsp+0x8]   -> pointer to the data
  ; [rsp+0x10]  -> pointer to the new node

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x10], 0

  test  rdi, rdi
  jz    .error

  ; create a node
  mov   rdi, [rsp+0x8]
  call  _linked_list_new_node
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; set the current head as node's next
  mov   rdi, [rsp+0x10]
  mov   rdx, [rsp]
  mov   rsi, [rdx]
  call  _linked_list_set_node_next
  cmp   byte [_linked_list_errno], TRUE
  je    .error

  ; set the node at the new head
  mov   rax, [rsp+0x10]
  mov   rdi, [rsp]
  mov   [rdi], rax

  mov   rax, SUCCESS_CODE

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

; adds a node to the end of the linked list
; @param  rdi: pointer to the pointer to the head
; @param  rsi: pointer to the data
; @return rax: return code
linked_list_insert_at_end:
  sub   rsp, 0x20

  ; STACK USAGE
  ; [rsp]       -> pointer to the pointer to the head
  ; [rsp+0x8]   -> pointer to the data
  ; [rsp+0x10]  -> pointer to the new node
  ; [rsp+0x18]  -> temp node

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x10], 0

  test  rdi, rdi
  jz    .error

  ; create a node
  mov   rdi, [rsp+0x8]
  call  _linked_list_new_node
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  mov   rdi, [rsp]
  mov   rsi, [rdi]
  mov   [rsp+0x18], rsi

  cmp   qword [rdi], 0
  jne   .loop

  ; head is null
  mov   [rdi], rax
  mov   rax, SUCCESS_CODE

  jmp   .return

.loop:
  ; get temp next
  mov   rdi, [rsp+0x18]
  call  _linked_list_get_node_next
  cmp   byte [_linked_list_errno], TRUE
  je    .error

  ; if next is null, insert node
  test  rax, rax
  jz    .insert_node

  ; temp = temp->next
  mov   [rsp+0x18], rax

  jmp   .loop

.insert_node:
  mov   rdi, [rsp+0x18]
  mov   rsi, [rsp+0x10]
  call  _linked_list_set_node_next
  cmp   byte [_linked_list_errno], TRUE
  je    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rdi, [rsp+0x10]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
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

; sets the next property of a node
; @param  rdi: pointer to the node
; @param  rsi: pointer to next node
; @return rax: return code
_linked_list_set_node_next:
  test  rdi, rdi
  jz    .error

  mov   [rdi+NODE_OFF_NEXT], rsi

  mov   rax, SUCCESS_CODE
  mov   byte [_linked_list_errno], FALSE

  jmp   .return

.error:
  mov   byte [_linked_list_errno], TRUE

.return:
  ret

; returns the data property of a node
; @param  rdi: pointer to the node
; @return rax: pointer to next node
_linked_list_get_node_next:
  test  rdi, rdi
  jz    .error

  mov   rax, [rdi+NODE_OFF_NEXT]
  mov   byte [_linked_list_errno], FALSE

  jmp   .return

.error:
  mov   byte [_linked_list_errno], TRUE

.return:
  ret

; sets the data property of a node
; @param  rdi: pointer to the node
; @param  rsi: data
; @return rax: return code
_linked_list_set_node_data:
  test  rdi, rdi
  jz    .error

  mov   [rdi+NODE_OFF_DATA], rsi

  mov   rax, SUCCESS_CODE
  mov   byte [_linked_list_errno], FALSE

  jmp   .return

.error:
  mov   byte [_linked_list_errno], TRUE

.return:
  ret

; returns the data property of a node
; @param  rdi: pointer to the node
; @return rax: data
_linked_list_get_node_data:
  test  rdi, rdi
  jz    .error

  mov   rax, [rdi+NODE_OFF_DATA]
  mov   byte [_linked_list_errno], FALSE

  jmp   .return

.error:
  mov   byte [_linked_list_errno], TRUE

.return:
  ret

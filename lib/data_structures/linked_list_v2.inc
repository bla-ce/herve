section .bss

_node_t:
  .data resq 1
  .next resq 1
_node_t_end:

section .data

NODE_STRUCT_LEN equ _node_t_end - _node_t

NODE_OFF_DATA equ 0x0
NODE_OFF_NEXT equ NODE_OFF_DATA + 0x8

linked_list_errno  db 0

section .text

; creates a new node
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
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, NO_ARG
  call  _linked_list_set_node_next
  cmp   byte [linked_list_errno], TRUE
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

; adds a node at the beginning of the linked list
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
  cmp   byte [linked_list_errno], TRUE
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

; adds a node at the end of the linked list
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
  call  linked_list_get_node_next
  cmp   byte [linked_list_errno], TRUE
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
  cmp   byte [linked_list_errno], TRUE
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

; removes the first node of the linked list
; the data is also free with a specific function if passed in rsi
; @param  rdi: pointer to pointer to the head
; @param  rsi: (optional) free function for the data
; @return rax: return code
linked_list_delete_from_first:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> pointer to the pointer to the head
  ; [rsp+0x8]   -> optional free func for the data
  ; [rsp+0x10]  -> temp

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  test  rdi, rdi
  jz    .error

  ; check if the list is already empty
  mov   rax, SUCCESS_CODE
  mov   rsi, [rdi]
  test  rsi, rsi
  jz    .return

  ; save temp
  mov   [rsp+0x10], rsi

  mov   rdi, rsi
  call  linked_list_get_node_next
  cmp   byte [linked_list_errno], TRUE
  je    .error

  ; *head = temp->next;
  mov   rdi, [rsp]
  mov   [rdi], rax

  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp+0x8]
  call  _linked_list_free_node
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; removes the last node of the linked list
; the data is also free with a specific function if passed in rsi
; @param  rdi: pointer to pointer to the head
; @param  rsi: (optional) free function for the data
; @return rax: return code
linked_list_delete_from_end:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> pointer to the pointer to the head
  ; [rsp+0x8]   -> optional free func for the data
  ; [rsp+0x10]  -> temp

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  test  rdi, rdi
  jz    .error

  ; check if the list is already empty
  mov   rax, SUCCESS_CODE
  mov   rsi, [rdi]
  test  rsi, rsi
  jz    .return

  ; save temp
  mov   [rsp+0x10], rsi

  ; check if temp->next is empty
  mov   rdi, [rsp+0x10]
  call  linked_list_get_node_next
  cmp   byte [linked_list_errno], TRUE
  je    .error

  test  rax, rax
  jnz   .loop

  ; linked list is one node long
  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp+0x8]
  call  _linked_list_free_node
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   qword [rdi], 0
  mov   rax, SUCCESS_CODE
  jmp   .return

.loop:
  mov   rdi, [rsp+0x10]
  call  _linked_list_get_node_next_next
  cmp   byte [linked_list_errno], TRUE
  je    .error

  ; if temp->next->next is null, remove node
  test  rax, rax
  jz    .remove_node

  mov   rdi, [rsp+0x10]
  call  linked_list_get_node_next
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   [rsp+0x10], rax

  jmp   .loop

.remove_node:
  ; free(temp->next)
  mov   rdi, [rsp+0x10]
  call  linked_list_get_node_next
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rdi, rax
  mov   rsi, [rsp+0x8]
  call  _linked_list_free_node
  cmp   rax, 0
  jl    .error

  ; temp->next = 0
  mov   rdi, [rsp+0x10]
  xor   rsi, rsi
  call  _linked_list_set_node_next
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; removes the first node with a specific value in the linked list
; the data is also free with a specific function if passed in rsi
; @param  rdi: pointer to pointer to the head
; @param  rsi: value to look for
; @param  rdx: (optional) free function for the data
; @return rax: return code
linked_list_delete_from_value:
  sub   rsp, 0x28

  ; STACK USAGE
  ; [rsp]       -> pointer to the pointer to the head
  ; [rsp+0x8]   -> value to look for
  ; [rsp+0x10]  -> optional free func for the data
  ; [rsp+0x18]  -> temp
  ; [rsp+0x20]  -> temp->next->next

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx

  test  rdi, rdi
  jz    .error

  ; check if the list is already empty
  mov   rax, SUCCESS_CODE
  mov   rsi, [rdi]
  test  rsi, rsi
  jz    .return

  ; save temp
  mov   [rsp+0x18], rsi

  ; check first node
  mov   rdi, rsi
  call  linked_list_get_node_data
  cmp   byte [linked_list_errno], TRUE
  je    .error

  cmp   rax, [rsp+0x8]
  jne   .loop

  ; get next
  mov   rdi, [rsp+0x18]
  call  linked_list_get_node_next
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rdi, [rsp]
  mov   [rdi], rax

  ; free node
  mov   rdi, [rsp+0x18]
  mov   rsi, [rsp+0x10]
  call  _linked_list_free_node
  cmp   rax, 0
  jl    .error

  ; return
  mov   rax, SUCCESS_CODE
  jmp   .return

.loop:
  ; get next
  mov   rdi, [rsp+0x18]
  call  linked_list_get_node_next
  cmp   byte [linked_list_errno], TRUE
  je    .error

  ; check if temp data is equal to the target value
  mov   rdi, rax
  call  linked_list_get_node_data
  cmp   byte [linked_list_errno], TRUE
  je    .error

  cmp   rax, [rsp+0x8]
  je    .remove_node

  mov   rdi, [rsp+0x18]
  call  linked_list_get_node_next
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   [rsp+0x18], rax

  jmp   .loop

.remove_node:
  ; get (temp->next->next)
  mov   rdi, [rsp+0x18]
  call  _linked_list_get_node_next_next
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   [rsp+0x20], rax

  ; free(temp->next)
  mov   rdi, [rsp+0x18]
  call  linked_list_get_node_next
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rdi, rax
  mov   rsi, [rsp+0x10]
  call  _linked_list_free_node
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x18]
  mov   rsi, [rsp+0x20]
  call  _linked_list_set_node_next
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x28
  ret

; frees all the node from the linked list
; the data is also free with a specific function if passed in rsi
; @param  rdi: pointer to the head
; @param  rsi: (optional) free function for the data
; @return rax: return code
linked_list_free_v2:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> pointer to the head
  ; [rsp+0x8]   -> optional free func for the data
  ; [rsp+0x10]  -> temp

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  test  rdi, rdi
  jz    .error

  ; check if the list is already empty
  mov   rax, SUCCESS_CODE
  mov   rsi, [rdi]
  test  rsi, rsi
  jz    .return

  ; save temp
  mov   [rsp+0x10], rsi

.loop:
  mov   rdi, [rsp+0x10]
  call  linked_list_get_node_next
  cmp   byte [linked_list_errno], TRUE
  je    .error

  ; *head = temp->next;
  mov   rdi, [rsp]
  mov   [rdi], rax

  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp+0x8]
  call  _linked_list_free_node
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, [rdi]

  mov   [rsp+0x10], rsi

  test  rsi, rsi
  jnz   .loop

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; frees a node from the linked list
; the data is also free with a specific function if passed in rsi
; @param  rdi: pointer to the node
; @param  rsi: (optional) free function for the data
; @return rax: return code
_linked_list_free_node:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> pointer to the node
  ; [rsp+0x8] -> free function for the data

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  test  rdi, rdi
  jz    .error

  ; if a free function is passed, we free the data
  mov   rdi, [rsp+0x8]
  test  rdi, rdi
  jz    .skip_free_func

  mov   rdi, [rsp]
  call  linked_list_get_node_data
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rdi, rax
  mov   rsi, [rsp+0x8]
  call  rsi

.skip_free_func:
  ; free temp
  mov   rdi, [rsp]
  call  free
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
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
  mov   byte [linked_list_errno], FALSE

  jmp   .return

.error:
  mov   byte [linked_list_errno], TRUE

.return:
  ret

; returns the next next property of a node
; @param  rdi: pointer to the node
; @return rax: pointer to next next node
_linked_list_get_node_next_next:
  test  rdi, rdi
  jz    .error

  mov   rsi, [rdi+NODE_OFF_NEXT]
  test  rsi, rsi
  jz    .error

  mov   rax, [rsi+NODE_OFF_NEXT]
  mov   byte [linked_list_errno], FALSE

  jmp   .return

.error:
  mov   byte [linked_list_errno], TRUE

.return:
  ret

; returns the next property of a node
; @param  rdi: pointer to the node
; @return rax: pointer to next node
linked_list_get_node_next:
  test  rdi, rdi
  jz    .error

  mov   rax, [rdi+NODE_OFF_NEXT]
  mov   byte [linked_list_errno], FALSE

  jmp   .return

.error:
  mov   byte [linked_list_errno], TRUE

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
  mov   byte [linked_list_errno], FALSE

  jmp   .return

.error:
  mov   byte [linked_list_errno], TRUE

.return:
  ret

; returns the data property of a node
; @param  rdi: pointer to the node
; @return rax: data
linked_list_get_node_data:
  test  rdi, rdi
  jz    .error

  mov   rax, [rdi+NODE_OFF_DATA]
  mov   byte [linked_list_errno], FALSE

  jmp   .return

.error:
  mov   byte [linked_list_errno], TRUE

.return:
  ret

global _start

%include "lib.inc"

section .bss

section .data

head dq 0

section .text

_start:
  mov   rdi, head
  mov   rsi, NO_ARG
  call  linked_list_delete_from_first
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rdi, head
  mov   rsi, 8
  call  linked_list_insert_at_first
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rax, [head]
  mov   rdi, [rax+NODE_OFF_DATA]
  mov   rsi, 8
  call  assert_equal

  mov   rax, [head]
  mov   rdi, [rax+NODE_OFF_NEXT]
  call  assert_is_zero

  mov   rdi, head
  mov   rsi, 10
  call  linked_list_insert_at_first
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rax, [head]
  mov   rdi, [rax+NODE_OFF_DATA]
  mov   rsi, 10
  call  assert_equal

  mov   rax, [head]
  mov   rsi, [rax+NODE_OFF_NEXT]
  mov   rdi, [rsi+NODE_OFF_DATA]
  mov   rsi, 8
  call  assert_equal

  mov   rdi, head
  mov   rsi, NO_ARG
  call  linked_list_delete_from_first
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rdi, head
  mov   rsi, NO_ARG
  call  linked_list_delete_from_first
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rdi, [head]
  call  assert_is_zero

  mov   rdi, head
  mov   rsi, 8
  call  linked_list_insert_at_end
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rax, [head]
  mov   rdi, [rax+NODE_OFF_DATA]
  mov   rsi, 8
  call  assert_equal

  mov   rax, [head]
  mov   rdi, [rax+NODE_OFF_NEXT]
  call  assert_is_zero

  mov   rdi, head
  mov   rsi, 10
  call  linked_list_insert_at_end
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rax, [head]
  mov   rdi, [rax+NODE_OFF_DATA]
  mov   rsi, 8
  call  assert_equal

  mov   rax, [head]
  mov   rsi, [rax+NODE_OFF_NEXT]
  mov   rdi, [rsi+NODE_OFF_DATA]
  mov   rsi, 10
  call  assert_equal

  mov   rax, [head]
  mov   rsi, [rax+NODE_OFF_NEXT]
  mov   rdi, [rsi+NODE_OFF_NEXT]
  call  assert_is_zero

  mov   rdi, head
  mov   rsi, NO_ARG
  call  linked_list_delete_from_first
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rax, [head]
  mov   rdi, [rax+NODE_OFF_DATA]
  mov   rsi, 10
  call  assert_equal

  mov   rdi, head
  mov   rsi, NO_ARG
  call  linked_list_delete_from_end
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rdi, [head]
  call  assert_is_zero

  mov   rdi, head
  mov   rsi, NO_ARG
  call  linked_list_delete_from_end
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rdi, [head]
  call  assert_is_zero

  mov   rdi, head
  mov   rsi, 10
  call  linked_list_insert_at_first
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rdi, head
  mov   rsi, 8
  call  linked_list_insert_at_first
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rdi, head
  mov   rsi, NO_ARG
  call  linked_list_delete_from_end
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rax, [head]
  mov   rdi, [rax+NODE_OFF_DATA]
  mov   rsi, 8
  call  assert_equal

  mov   rax, [head]
  mov   rdi, [rax+NODE_OFF_NEXT]
  call  assert_is_zero

  mov   rdi, head
  mov   rsi, 8
  call  linked_list_insert_at_first
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rdi, head
  mov   rsi, 14
  call  linked_list_insert_at_first
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rdi, head
  mov   rsi, 21
  call  linked_list_insert_at_first
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rax, [head]
  mov   rdi, [rax+NODE_OFF_DATA]
  mov   rsi, 21
  call  assert_equal

  mov   rdi, head
  mov   rsi, 14
  mov   rdx, NO_ARG
  call  linked_list_delete_from_value
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rax, [head]
  mov   rsi, [rax+NODE_OFF_NEXT]
  mov   rdi, [rsi+NODE_OFF_DATA]
  mov   rsi, 8
  call  assert_equal

  mov   rdi, head
  mov   rsi, 8
  mov   rdx, NO_ARG
  call  linked_list_delete_from_value
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rdi, head
  mov   rsi, 8
  mov   rdx, NO_ARG
  call  linked_list_delete_from_value
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rax, [head]
  mov   rdi, [rax+NODE_OFF_DATA]
  mov   rsi, 21
  call  assert_equal

  mov   rax, [head]
  mov   rdi, [rax+NODE_OFF_NEXT]
  call  assert_is_zero

  mov   rdi, head
  mov   rsi, 21
  mov   rdx, NO_ARG
  call  linked_list_delete_from_value
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rdi, [head]
  call  assert_is_zero

  xor   r9, r9

.loop:
  cmp   r9, 500
  jge   .loop_end

  mov   rdi, head
  mov   rsi, r9
  call  linked_list_insert_at_first
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rdi, head
  mov   rsi, r9
  mov   rdx, NO_ARG
  call  linked_list_delete_from_value
  cmp   byte [linked_list_errno], TRUE
  je    .error

  inc   r9

  jmp   .loop
.loop_end:

  mov   rdi, head
  mov   rsi, NO_ARG
  call  linked_list_free
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rdi, [mallocd]
  cmp   [freed], rdi
  jne   .error

  mov   rdi, SUCCESS_CODE
  call  exit

.error:
  mov   rdi, FAILURE_CODE
  call  exit

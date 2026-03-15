global _start

%include "lib.inc"

section .bss
dummy_t:
  .field1 resq 1
  .field2 resq 1
  .next   resq 1
dummy_t_end:

section .data

DUMMY_OFF_FIELD1  equ 0x0
DUMMY_OFF_FIELD2  equ 0x8
DUMMY_OFF_NEXT    equ 0x10

DUMMY_STRUCT_LEN equ dummy_t_end - dummy_t

head dq 0

d1 dq 0
d2 dq 0
d3 dq 0
d4 dq 0

section .text

_start:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp]   -> pointer to the linked list head
  mov   rdi, DUMMY_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [d1], rax

  mov   rdi, DUMMY_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [d2], rax

  mov   rdi, DUMMY_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [d3], rax

  mov   rdi, DUMMY_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [d4], rax

  ; d1
  mov   rdi, head
  mov   rsi, [d1]
  mov   rdx, DUMMY_OFF_NEXT
  call  linked_list_add_entry
  cmp   rax, 0
  jl    .error

  ; d1 -> d2
  mov   rdi, head
  mov   rsi, [d2]
  mov   rdx, DUMMY_OFF_NEXT
  call  linked_list_add_entry
  cmp   rax, 0
  jl    .error

  ; d1 -> d2 -> d3
  mov   rdi, head
  mov   rsi, [d3]
  mov   rdx, DUMMY_OFF_NEXT
  call  linked_list_add_entry
  cmp   rax, 0
  jl    .error

  mov   rdi, [head]
  mov   rsi, [d1]
  call  assert_equal
  cmp   rax, 0
  jl    .error

  ; d1 -> d3
  mov   rdi, head
  mov   rsi, [d2]
  mov   rdx, DUMMY_OFF_NEXT
  call  linked_list_remove_entry
  cmp   rax, 0
  jl    .error

  mov   rdi, [d1+DUMMY_OFF_NEXT]
  mov   rsi, [d3]
  call  assert_equal
  cmp   rax, 0
  jl    .error

  ; d1 -> d3 -> d4
  mov   rdi, head
  mov   rsi, [d4]
  mov   rdx, DUMMY_OFF_NEXT
  call  linked_list_add_entry
  cmp   rax, 0
  jl    .error

  ; d1 -> d3
  mov   rdi, head
  mov   rsi, [d4]
  mov   rdx, DUMMY_OFF_NEXT
  call  linked_list_remove_entry
  cmp   rax, 0
  jl    .error

  mov   rdi, [d3+DUMMY_OFF_NEXT]
  call  assert_is_zero
  cmp   rax, 0
  jl    .error

  ; d3
  mov   rdi, head
  mov   rsi, [d1]
  mov   rdx, DUMMY_OFF_NEXT
  call  linked_list_remove_entry
  cmp   rax, 0
  jl    .error

  mov   rdi, [head]
  mov   rsi, [d3]
  call  assert_equal
  cmp   rax, 0
  jl    .error

  ; null
  mov   rdi, head
  mov   rsi, [d3]
  mov   rdx, DUMMY_OFF_NEXT
  call  linked_list_remove_entry
  cmp   rax, 0
  jl    .error

  mov   rdi, [head]
  call  assert_is_zero
  cmp   rax, 0
  jl    .error

  mov   rdi, SUCCESS_CODE
  call  exit

.error:
  mov   rdi, FAILURE_CODE
  call  exit

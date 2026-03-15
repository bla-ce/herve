global _start

%include "lib.inc"

section .bss

section .data

head dq 0

section .text

_start:
  mov   rdi, head
  mov   rsi, 8
  call  linked_list_insert_at_first
  cmp   rax, 0
  jl    .error

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
  cmp   rax, 0
  jl    .error

  mov   rax, [head]
  mov   rdi, [rax+NODE_OFF_DATA]
  mov   rsi, 10
  call  assert_equal

  mov   rax, [head]
  mov   rsi, [rax+NODE_OFF_NEXT]
  mov   rdi, [rsi+NODE_OFF_DATA]
  mov   rsi, 8
  call  assert_equal

  mov   qword [head], 0

  mov   rdi, head
  mov   rsi, 8
  call  linked_list_insert_at_end
  cmp   rax, 0
  jl    .error

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
  cmp   rax, 0
  jl    .error

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

  mov   rdi, SUCCESS_CODE
  call  exit

.error:
  mov   rdi, FAILURE_CODE
  call  exit

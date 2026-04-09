global _start

%include "lib.inc"

section .text
_start:
  call  boeuf_init
  cmp   rax, 0
  jl    .error

  mov   [boeuf_buf], rax

  mov   rdi, rax
  call  boeuf_is_corrupted

  mov   rdi, rax
  mov   rsi, FALSE
  call  assert_equal

  ; corrupt the boeuf to test
  mov   rax, [boeuf_buf]
  mov   qword [rax+_BOEUF_OFF_MAGIC_VALUE], 0xBEEF

  mov   rdi, rax
  call  boeuf_is_corrupted

  mov   rdi, rax
  mov   rsi, TRUE
  call  assert_equal

  mov   rax, [boeuf_buf]
  mov   qword [rax+_BOEUF_OFF_MAGIC_VALUE], _BOEUF_MAGIC_VALUE

  mov   rdi, rax
  call  boeuf_get_max_cap

  mov   rdi, rax
  mov   rsi, _BOEUF_DEFAULT_MAX_CAPACITY
  call  assert_equal

  mov   rdi, [boeuf_buf]
  call  boeuf_free
  cmp   rax, 0
  jl    .error

  mov   rax, [mallocd]
  cmp   rax, [freed]
  jne   .error

  mov   rdi, SUCCESS_CODE
  call  exit

.error:
  mov   rdi, FAILURE_CODE
  call  exit

section .data

boeuf_buf dq 0

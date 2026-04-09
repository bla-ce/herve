global _start

%include "lib.inc"

section .text
_start:
  call  boeuf_init
  cmp   rax, 0
  jl    .error

  mov   [boeuf_buf], rax

  ; boeuf_is_corrupted returns FALSE for valid boeuf
  mov   rdi, [boeuf_buf]
  call  boeuf_is_corrupted

  mov   rdi, rax
  mov   rsi, FALSE
  call  assert_equal

  ; boeuf_is_corrupted returns TRUE for corrupted magic value
  mov   rax, [boeuf_buf]
  mov   qword [rax+_BOEUF_OFF_MAGIC_VALUE], 0xBEEF

  mov   rdi, rax
  call  boeuf_is_corrupted

  mov   rdi, rax
  mov   rsi, TRUE
  call  assert_equal

  ; restore magic value
  mov   rax, [boeuf_buf]
  mov   qword [rax+_BOEUF_OFF_MAGIC_VALUE], _BOEUF_MAGIC_VALUE

  mov   rdi, [boeuf_buf]
  mov   rsi, str_1
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  mov   rdi, [boeuf_buf]
  call  boeuf_get_len

  mov   rdi, rax
  mov   rsi, 12 ; "Hello, sir! " is 12 chars
  call  assert_equal

  ; boeuf_get_cap is non-zero after append
  mov   rdi, [boeuf_buf]
  call  boeuf_get_cap

  mov   rdi, rax
  call  assert_is_not_zero

  mov   rdi, [boeuf_buf]
  call  boeuf_to_str
  cmp   rax, 0
  jl    .error

  mov   [boeuf_str], rax

  mov   rdi, rax
  mov   rsi, str_1
  call  assert_string_equal

  mov   rdi, [boeuf_str]
  call  free
  cmp   rax, 0
  jl    .error

  mov   rdi, [boeuf_buf]
  mov   rsi, str_2
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  ; boeuf_get_len returns correct length after second append
  mov   rdi, [boeuf_buf]
  call  boeuf_get_len

  mov   rdi, rax
  mov   rsi, 24 ; "Hello, sir! How are you?" is 24 chars
  call  assert_equal

  mov   rdi, [boeuf_buf]
  call  boeuf_to_str
  cmp   rax, 0
  jl    .error

  mov   [boeuf_str], rax

  mov   rdi, rax
  mov   rsi, str_1_2
  call  assert_string_equal

  mov   rdi, [boeuf_str]
  call  free
  cmp   rax, 0
  jl    .error

  ; boeuf_clear resets length to 0
  mov   rdi, [boeuf_buf]
  call  boeuf_clear
  cmp   rax, 0
  jl    .error

  mov   rax, [boeuf_buf]
  mov   rdi, [rax+_BOEUF_OFF_LENGTH]
  call  assert_is_zero

  ; boeuf_clear sets first byte to null
  mov   rax, [boeuf_buf]
  mov   rsi, [rax+_BOEUF_OFF_DATA]
  movzx rdi, byte [rsi]
  call  assert_is_zero

  ; boeuf_get_cap is preserved after clear
  mov   rdi, [boeuf_buf]
  call  boeuf_get_cap

  mov   rdi, rax
  call  assert_is_not_zero

  ; boeuf_append_str works after clear
  mov   rdi, [boeuf_buf]
  mov   rsi, str_2
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  mov   rdi, [boeuf_buf]
  call  boeuf_to_str
  cmp   rax, 0
  jl    .error

  mov   [boeuf_str], rax

  mov   rdi, rax
  mov   rsi, str_2
  call  assert_string_equal

  mov   rdi, [boeuf_str]
  call  free
  cmp   rax, 0
  jl    .error

  mov   rdi, [boeuf_buf]
  call  boeuf_clear
  cmp   rax, 0
  jl    .error

  mov   rdi, [boeuf_buf]
  mov   rsi, str_1
  mov   rdx, 5 ; only "Hello"
  call  boeuf_nappend
  cmp   rax, 0
  jl    .error

  mov   rdi, [boeuf_buf]
  call  boeuf_get_len

  mov   rdi, rax
  mov   rsi, 5
  call  assert_equal

  mov   rdi, [boeuf_buf]
  call  boeuf_to_str
  cmp   rax, 0
  jl    .error

  mov   [boeuf_str], rax

  mov   rdi, rax
  mov   rsi, str_hello
  call  assert_string_equal

  mov   rdi, [boeuf_str]
  call  free
  cmp   rax, 0
  jl    .error

  ; boeuf_nappend with null source returns success (no-op)
  mov   rdi, [boeuf_buf]
  mov   rsi, 0
  mov   rdx, 10
  call  boeuf_nappend

  mov   rdi, rax
  mov   rsi, SUCCESS_CODE
  call  assert_equal

  ; boeuf_nappend with zero length returns success (no-op)
  mov   rdi, [boeuf_buf]
  mov   rsi, str_1
  mov   rdx, 0
  call  boeuf_nappend

  mov   rdi, rax
  mov   rsi, SUCCESS_CODE
  call  assert_equal

  ; restore magic value for cleanup
  mov   rax, [boeuf_buf]
  mov   qword [rax+_BOEUF_OFF_MAGIC_VALUE], _BOEUF_MAGIC_VALUE

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

boeuf_str dq 0

str_1     db "Hello, sir! ", NULL_CHAR
str_2     db "How are you?", NULL_CHAR

str_1_2   db "Hello, sir! How are you?", NULL_CHAR

str_hello db "Hello", NULL_CHAR

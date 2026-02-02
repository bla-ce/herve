global _start

%include "lib.inc"

section .text
_start:
  ; create boeuf
  mov   rdi, init_msg
  call  boeuf_create
  cmp   rax, 0
  jl    .error

  mov   [boeuf_buf], rax

  ; check boeuf content
  mov   rdi, rax
  mov   rsi, init_msg
  call  assert_string_equal

  ; check errno
  mov   rdi, [boeuf_errno]
  mov   rsi, BOEUF_NO_ERR
  call  assert_equal

  ; check length and size
  mov   rsi, [boeuf_buf]
  mov   rdi, qword [rsi - BOEUF_METADATA_LEN + BOEUF_METADATA_OFF_SIZE]
  mov   rsi, init_msg_len
  call  assert_equal

  mov   rsi, [boeuf_buf]
  mov   rdi, qword [rsi - BOEUF_METADATA_LEN + BOEUF_METADATA_OFF_LENGTH]
  mov   rsi, init_msg_len
  call  assert_equal

  mov   rdi, [boeuf_buf]
  mov   rsi, second_msg
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  mov   [boeuf_buf], rax

  ; check boeuf content
  mov   rdi, rax
  mov   rsi, msg_1
  call  assert_string_equal

  ; check errno
  mov   rdi, [boeuf_errno]
  mov   rsi, BOEUF_NO_ERR
  call  assert_equal

  ; check boeuf len and size
  mov   rsi, [boeuf_buf]
  mov   rdi, qword [rsi - BOEUF_METADATA_LEN + BOEUF_METADATA_OFF_LENGTH]
  mov   rsi, msg1_len
  call  assert_equal

  mov   rsi, [boeuf_buf]
  mov   rdi, qword [rsi - BOEUF_METADATA_LEN + BOEUF_METADATA_OFF_SIZE]
  mov   rsi, 16 ; size is doubled 2x (4 (orig size) - 8 - 16)
  call  assert_equal

  mov   rdi, [boeuf_buf]
  mov   rsi, init_msg
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  mov   [boeuf_buf], rax

  ; check boeuf content
  mov   rdi, rax
  mov   rsi, msg_2
  call  assert_string_equal

  ; check errno
  mov   rdi, [boeuf_errno]
  mov   rsi, BOEUF_NO_ERR
  call  assert_equal

  ; check boeuf len and size
  mov   rsi, [boeuf_buf]
  mov   rdi, qword [rsi - BOEUF_METADATA_LEN + BOEUF_METADATA_OFF_LENGTH]
  mov   rsi, msg2_len
  call  assert_equal

  mov   rsi, [boeuf_buf]
  mov   rdi, qword [rsi - BOEUF_METADATA_LEN + BOEUF_METADATA_OFF_SIZE]
  mov   rsi, 16 ; length is doubled 2x (4 (orig size) - 8 - 16)
  call  assert_equal

  mov   rdi, [boeuf_buf]
  mov   rsi, third_msg
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  mov   [boeuf_buf], rax

  ; check boeuf content
  mov   rdi, rax
  mov   rsi, msg_3
  call  assert_string_equal

  ; check errno
  mov   rdi, [boeuf_errno]
  mov   rsi, BOEUF_NO_ERR
  call  assert_equal

  ; check boeuf len and size
  mov   rsi, [boeuf_buf]
  mov   rdi, qword [rsi - BOEUF_METADATA_LEN + BOEUF_METADATA_OFF_LENGTH]
  mov   rsi, msg3_len
  call  assert_equal

  mov   rsi, [boeuf_buf]
  mov   rdi, qword [rsi - BOEUF_METADATA_LEN + BOEUF_METADATA_OFF_SIZE]
  mov   rsi, 32 ; size is doubled once
  call  assert_equal

  ; change length of boeuf with invalid value
  mov   rdi, [boeuf_buf]
  mov   rsi, 500
  call  boeuf_set_len
  cmp   rax, 0
  jg    .error

  ; append string with invalid length
  mov   rdi, [boeuf_buf]
  mov   rsi, init_msg
  mov   rdx, 500
  call  boeuf_nappend
  cmp   rax, 0
  jg    .error

  ; append string with 0 length -> it should return the same boeuf
  mov   rdi, [boeuf_buf]
  mov   rsi, init_msg
  mov   rdx, 0
  call  boeuf_nappend
  cmp   rax, 0
  jl    .error

  cmp   [boeuf_buf], rax
  jne   .error

  ; change length of boeuf
  mov   rdi, [boeuf_buf]
  mov   rsi, 3
  call  boeuf_set_len
  cmp   rax, 0
  jl    .error

  mov   rdi, [boeuf_buf]
  call  boeuf_len
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, 3
  call  assert_equal

  mov   rdi, [boeuf_buf]
  mov   rsi, init_msg
  call  assert_string_equal
  cmp   rax, 0
  jl    .error

  mov   rsi, [boeuf_buf]
  mov   rdi, qword [rsi - BOEUF_METADATA_LEN + BOEUF_METADATA_OFF_SIZE]
  mov   rsi, 32 ; size should be equal
  call  assert_equal

  mov   rdi, [boeuf_buf]
  call  boeuf_reset
  cmp   rax, 0
  jl    .error

  mov   rsi, [boeuf_buf]
  mov   rdi, qword [rsi - BOEUF_METADATA_LEN + BOEUF_METADATA_OFF_SIZE]
  mov   rsi, 32 ; size should be equal
  call  assert_equal

  mov   rdi, [boeuf_buf]
  call  boeuf_len
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, 0
  call  assert_equal

  mov   rdi, qword [boeuf_buf]
  call  boeuf_free
  cmp   rax, 0
  jl    .error

  mov   rdi, qword [mallocd]
  mov   rsi, qword [freed]
  call  assert_equal

  mov   rdi, SUCCESS_CODE
  call  exit

.error:
  mov   rdi, FAILURE_CODE
  call  exit

section .data
boeuf_buf dq 0

init_msg      db " | ", NULL_CHAR
init_msg_len  equ $ - init_msg

second_msg      db "127.0.0.1", NULL_CHAR
second_msg_len  equ $ - second_msg

msg_1     db " | 127.0.0.1", NULL_CHAR
msg1_len  equ $ - msg_1

msg_2     db " | 127.0.0.1 | ", NULL_CHAR
msg2_len  equ $ - msg_2

third_msg     db "200 OK", NULL_CHAR
third_msg_len equ $ - third_msg

msg_3     db " | 127.0.0.1 | 200 OK", NULL_CHAR
msg3_len  equ $ - msg_3

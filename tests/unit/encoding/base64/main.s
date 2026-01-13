global _start

%include "lib.inc"

section .text
_start:
  mov   rdi, dec_str_1
  call  base64_encode
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, enc_str_1
  call  assert_string_equal

  mov   rdi, dec_str_2
  call  base64_encode
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, enc_str_2
  call  assert_string_equal

  mov   rdi, dec_str_3
  call  base64_encode
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, enc_str_3
  call  assert_string_equal

  mov   rdi, dec_str_4
  call  base64_encode
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, enc_str_4
  call  assert_string_equal

  mov   rdi, dec_str_5
  call  base64_encode
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, enc_str_5
  call  assert_string_equal

  mov   rdi, dec_str_6
  call  base64_encode
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, enc_str_6
  call  assert_string_equal

  mov   rdi, enc_str_1
  call  base64_decode
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, dec_str_1
  call  assert_string_equal

  mov   rdi, enc_str_2
  call  base64_decode
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, dec_str_2
  call  assert_string_equal

  mov   rdi, enc_str_3
  call  base64_decode
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, dec_str_3
  call  assert_string_equal

  mov   rdi, enc_str_4
  call  base64_decode
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, dec_str_4
  call  assert_string_equal

  mov   rdi, enc_str_5
  call  base64_decode
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, dec_str_5
  call  assert_string_equal

  mov   rdi, enc_str_6
  call  base64_decode
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, dec_str_6
  call  assert_string_equal

  mov   rdi, SUCCESS_CODE
  call  exit

.error:
  mov   rdi, FAILURE_CODE
  call  exit

section .data

dec_str_1 db "hello, sir!", NULL_CHAR
enc_str_1 db "aGVsbG8sIHNpciE=", NULL_CHAR

dec_str_2 db "I am decoded", NULL_CHAR
enc_str_2 db "SSBhbSBkZWNvZGVk", NULL_CHAR

dec_str_3 db "I am decoded, interesting", NULL_CHAR
enc_str_3 db "SSBhbSBkZWNvZGVkLCBpbnRlcmVzdGluZw==", NULL_CHAR

dec_str_4 db "This is another test.", NULL_CHAR
enc_str_4 db "VGhpcyBpcyBhbm90aGVyIHRlc3Qu", NULL_CHAR

dec_str_5 db "Encode me, please", NULL_CHAR
enc_str_5 db "RW5jb2RlIG1lLCBwbGVhc2U=", NULL_CHAR

dec_str_6 db "Encode me!", NULL_CHAR
enc_str_6 db "RW5jb2RlIG1lIQ==", NULL_CHAR

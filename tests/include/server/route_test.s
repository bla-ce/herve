global _start

%include "herve.inc"

section .data

url_root    db "/", NULL_CHAR
url_valid   db "/e", NULL_CHAR
url_valid2  db "/hello", NULL_CHAR

url_without_leading_slash   db "e", NULL_CHAR
url_without_leading_slash2  db "hello", NULL_CHAR

url_with_trailing_slash   db "e/", NULL_CHAR
url_with_trailing_slash2  db "hello/", NULL_CHAR

url_with_uppercase  db "HeLLo", NULL_CHAR

section .text

_start:
  mov   rdi, url_root
  call  route_normalise_url
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, url_root
  call  assert_string_equal

  mov   rdi, url_without_leading_slash
  call  route_normalise_url
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, url_valid
  call  assert_string_equal

  mov   rdi, url_without_leading_slash2
  call  route_normalise_url
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, url_valid2
  call  assert_string_equal

  mov   rdi, url_with_trailing_slash
  call  route_normalise_url
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, url_valid
  call  assert_string_equal

  mov   rdi, url_with_trailing_slash2
  call  route_normalise_url
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, url_valid2
  call  assert_string_equal

  mov   rdi, url_with_uppercase
  call  route_normalise_url
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, url_valid2
  call  assert_string_equal

  mov   rdi, SUCCESS_CODE
  call  exit

.error:
  mov   rdi, FAILURE_CODE
  call  exit

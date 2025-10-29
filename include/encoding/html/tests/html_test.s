global _start

%include "herve.inc"

section .text
_start:
  sub   rsp, 0x10

  mov   rdi, str_no_escaping
  call  escape_html

  mov   [rsp], rax

  mov   rdi, [rsp]
  call  println

  mov   rdi, [rsp]
  mov   rsi, str_no_escaping
  call  strcmp
  cmp   rax, 0
  jl    .error

  cmp   rax, TRUE
  jne   .error

  mov   rdi, str_quote
  call  escape_html

  mov   [rsp+0x8], rax

  mov   rdi, [rsp+0x8]
  call  println

  mov   rdi, [rsp+0x8]
  mov   rsi, str_quote_esc
  call  strcmp
  cmp   rax, 0
  jl    .error

  cmp   rax, TRUE
  jne   .error

  mov   rdi, full
  call  escape_html

  mov   [rsp+0x8], rax

  mov   rdi, [rsp+0x8]
  call  println

  mov   rdi, [rsp+0x8]
  mov   rsi, full_esc
  call  strcmp
  cmp   rax, 0
  jl    .error

  cmp   rax, TRUE
  jne   .error

  mov   rdi, start
  call  escape_html

  mov   [rsp+0x8], rax

  mov   rdi, [rsp+0x8]
  call  println

  mov   rdi, [rsp+0x8]
  mov   rsi, start_esc
  call  strcmp
  cmp   rax, 0
  jl    .error

  cmp   rax, TRUE
  jne   .error

  mov   rdi, row
  call  escape_html

  mov   [rsp+0x8], rax

  mov   rdi, [rsp+0x8]
  call  println

  mov   rdi, [rsp+0x8]
  mov   rsi, row_esc
  call  strcmp
  cmp   rax, 0
  jl    .error

  cmp   rax, TRUE
  jne   .error

  mov   rdi, SUCCESS_CODE
  call  exit

.error:
  mov   rdi, FAILURE_CODE
  call  exit

section .data
  str_no_escaping db "Hello, this string does not need to be escaped! HAHA", NULL_CHAR

  str_quote     db "Some of the characters here don't need to be escaped! HAHA", NULL_CHAR
  str_quote_esc db "Some of the characters here don&apos;t need to be escaped! HAHA", NULL_CHAR

  full      db `List of characters: ", ', &, <, >`, NULL_CHAR
  full_esc  db `List of characters: &quot;, &apos;, &amp;, &lt;, &gt;`, NULL_CHAR

  start     db "< starting with escape character", NULL_CHAR
  start_esc db "&lt; starting with escape character", NULL_CHAR

  row     db "<> two characters in a row", NULL_CHAR
  row_esc db "&lt;&gt; two characters in a row", NULL_CHAR

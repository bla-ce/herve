global _start

%include "herve.inc"
%include "test.inc"
%include "os.inc"

section .text
_start:
  mov   rdi, 1
  call  assert_is_not_zero

  mov   rdi, SUCCESS_CODE
  call  exit
  
.error:
  mov   rdi, FAILURE_CODE
  call  exit

section .data
  str_1 db "Hello, World!", NULL_CHAR
  str_2 db "Hello, World!", NULL_CHAR

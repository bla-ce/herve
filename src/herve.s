global _start

%include "herve.inc"
%include "os.inc"

section .text

_start:
  mov   rdi, SUCCESS_CODE
  call  exit

.error:
  mov   rdi, FAILURE_CODE
  call  exit


global _start

%include "herve.inc"
%include "os.inc"

section .text

_start:
  mov   rdi, 69
  call  exit


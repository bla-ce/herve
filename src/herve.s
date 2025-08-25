global _start

%include "herve.inc"
%include "os.inc"

section .data
  MIN_CLI_ARGS equ 2

section .text
_start:
  sub   rsp, 0x8

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the cli args
  ; [rsp+0x8]   -> argv
  ; [rsp+0x10]  -> pointer to argc

  ; read cli args
  mov   rdi, [rsp+0x8]    ; offset 0x8 because we sub above
  cmp   rdi, MIN_CLI_ARGS ; minimum of 2 arguments "./herve <cmd>"
  jl    .error

  mov   rsi, [rsp+0x10]
  call  parse_cli
  cmp   rax, 0
  jl    .error

  mov   [rsp], rax

  mov   rdi, [rsp]
  call  free
  cmp   rax, 0
  jl    .error

  add   rsp, 0x8

  mov   rdi, SUCCESS_CODE
  call  exit

.error:
  add   rsp, 0x8

  mov   rdi, FAILURE_CODE
  call  exit


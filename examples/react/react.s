global _start

%include "herve.inc"

section .text
_start:
  sub   rsp, 0x8

  mov   rdi, 1337
  call  server_init
  cmp   rax, 0
  jl    .error

  mov   [rsp], rdi

  mov   rdi, [rsp]
  mov   rsi, dist_path
  mov   rdx, TRUE
  call  add_dir_route
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  call  run_server
  cmp   rax, 0
  jl    .error

  jmp   .exit

.error:
  mov   rax, SYS_EXIT
  mov   rdi, FAILURE_CODE
  syscall

.exit:
  mov   rax, SYS_EXIT
  mov   rdi, SUCCESS_CODE
  syscall

section .data
  dist_path db "examples/react/frontend/dist", NULL_CHAR


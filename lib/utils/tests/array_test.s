global _start

%include "herve.inc"
%include "os.inc"

section .text

_start:
  sub   rsp, 0x8

  ; *** STACK USAGE *** ;
  ; [rsp]   -> pointer to the array struct

  mov   rdi, 4
  mov   rsi, 8
  call  array_create
  cmp   rax, 0
  jl    .error

  mov   [rsp], rax

  mov   rdi, [rsp]
  mov   rsi, 420
  call  array_push
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, 1024
  call  array_push
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, -100
  call  array_push
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, 50
  call  array_push
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, 72
  call  array_push
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  call  array_free
  cmp   rax, 0
  jl    .error

  mov   rdi, qword [mallocd]
  mov   rsi, qword [freed]

  cmp   rdi, rsi
  jne   .error

  mov   rdi, SUCCESS_CODE
  call  exit

.error:
  mov   rdi, FAILURE_CODE
  call  exit
    

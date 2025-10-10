global _start

%include "herve.inc"
%include "os.inc"

section .data

value1  dq 421
value2  dq 422
value3  dq 423
value4  dq 424
value5  dq 425

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
  mov   rsi, value1
  call  array_push
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, value2
  call  array_push
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, value3
  call  array_push
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, value4
  call  array_push
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, value5
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
    

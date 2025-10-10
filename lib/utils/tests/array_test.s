global _start

%include "herve.inc"
%include "os.inc"

section .data

value1  dw 421
value2  dw 422
value3  dw 423
value4  dw 424
value5  dw 425

section .text

_start:
  sub   rsp, 0x8

  ; *** STACK USAGE *** ;
  ; [rsp]   -> pointer to the array struct

  mov   rdi, 4
  mov   rsi, 2
  call  array_create
  cmp   rax, 0
  jl    .error

  mov   [rsp], rax

  ; this one should fail
  mov   rdi, [rsp]
  call  array_pop
  cmp   rax, FAILURE_CODE
  jne   .error

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
  mov   rsi, 3
  call  array_get

  mov   rdi, [rsp]
  mov   rsi, value5
  call  array_push
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  call  array_pop
  cmp   rax, 0
  jl    .error

  cmp   rax, qword [value5]
  jne   .error

  ; this one should fail 
  mov   rdi, [rsp]
  mov   rsi, 10
  call  array_get
  cmp   rax, FAILURE_CODE
  jne   .error

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
    

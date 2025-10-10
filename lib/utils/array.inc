section .bss

array_t:
  elements  resq 1
  size      resq 1
  el_size   resq 1
array_t_end:

section .data

ARRAY_T_LEN equ array_t_end - array_t

; offsets

ARRAY_OFF_EL      equ 0
ARRAY_OFF_SIZE    equ ARRAY_OFF_EL + 0x8
ARRAY_OFF_EL_SIZE equ ARRAY_OFF_SIZE + 0x8

section .text

; creates an array of n elements
; @param  rdi: initial number of elements
; @param  rsi: size of elements
; @return rax: pointer to the array
array_create:
  sub   rsp, 0x20

  ; *** STACK USAGE *** ;
  ; [rsp]       -> number of elements 
  ; [rsp+0x8]   -> size of elements 
  ; [rsp+0x10]  -> pointer to the array
  ; [rsp+0x18]  -> pointer to the elements

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x10], 0
  mov   qword [rsp+0x18], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; malloc array struct
  mov   rdi, ARRAY_T_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  mov   rdi, [rsp]
  mov   [rax+ARRAY_OFF_SIZE], rdi

  mov   rdi, [rsp+0x8]
  mov   [rax+ARRAY_OFF_EL_SIZE], rdi

  ; calloc n * size
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  call  calloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  mov   rdi, [rsp+0x10]
  mov   [rdi+ARRAY_OFF_EL], rax

  mov   rax, rdi

  jmp   .return

.error:
  mov   rdi, [rsp+0x10]
  test  rdi, rdi
  jz    .free_array_struct

  call  free

.free_array_struct:
  mov   rdi, [rsp+0x18]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret

; frees the array struct and associated resources
; @param  rdi: pointer to the array struct
; @return rax: return code
array_free:
  sub   rsp, 0x8

  ; *** STACK USAGE *** ;
  ; [rsp] -> pointer to the array struct

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

  ; free array
  mov   rax, [rsp]
  mov   rdi, [rax+ARRAY_OFF_EL]
  call  free
  cmp   rax, 0
  jl    .error

  ; free array struct
  mov   rdi, [rsp]
  call  free
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

array_push:
  ret

array_pop:
  ret


array_get:
  ret


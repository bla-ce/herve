section .bss

array_t:
  elements  resq 1
  size      resq 1
  el_size   resq 1
  index     resq 1
array_t_end:

section .data

ARRAY_T_LEN equ array_t_end - array_t

; offsets
ARRAY_OFF_EL      equ 0
ARRAY_OFF_SIZE    equ ARRAY_OFF_EL + 0x8
ARRAY_OFF_EL_SIZE equ ARRAY_OFF_SIZE + 0x8
ARRAY_OFF_INDEX   equ ARRAY_OFF_EL_SIZE + 0x8

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

  mov   qword [rax+ARRAY_OFF_INDEX], 0

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

; pushes an element to the array
; @param  rdi: pointer to the array struct
; @param  rsi: pointer to the new element
; @return rax: return code
array_push:
  sub   rsp, 0x28

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the array struct
  ; [rsp+0x8]   -> pointer to the new element
  ; [rsp+0x10]  -> old size of the array
  ; [rsp+0x18]  -> pointer to old array
  ; [rsp+0x20]  -> pointer to new array

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x20], 0

  cmp   rdi, 0
  jle   .error

  ; check if array needs to be increased in size
  mov   rdi, [rsp]
  mov   rax, [rdi+ARRAY_OFF_SIZE]
  mov   rsi, [rdi+ARRAY_OFF_INDEX]
  cmp   rsi, rax
  jl    .insert

  ; save old size of the array
  mov   [rsp+0x10], rax

  ; get new size of the array
  xor   rdx, rdx
  mov   rbx, 2
  mul   rbx

  ; save new size into array struct
  mov   rdi, [rsp]
  mov   [rdi+ARRAY_OFF_SIZE], rax

  ; create new array of element
  mov   rsi, [rdi+ARRAY_OFF_EL_SIZE]
  mov   rdi, rax
  call  calloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  ; copy content
  mov   rdi, [rsp]

  xor   rdx, rdx
  mov   rbx, [rdi+ARRAY_OFF_EL_SIZE]
  mov   rax, [rsp+0x10] ; old size of the array
  mul   rbx

  mov   rcx, rax

  ; get old array
  mov   rdi, [rsp]
  mov   rsi, [rdi+ARRAY_OFF_EL]
  mov   [rsp+0x18], rsi

  ; new array
  mov   rdi, [rsp+0x20]
  rep   movsb

  ; update array element
  mov   rdi, [rsp+0x20]
  mov   rax, [rsp]
  mov   [rax+ARRAY_OFF_EL], rdi

  ; free prev array
  mov   rdi, [rsp+0x18]
  call  free
  cmp   rax, 0
  jl    .error

.insert:
  xor   rdx, rdx

  mov   rsi, [rsp]
  mov   rax, [rsi+ARRAY_OFF_EL_SIZE]
  mov   rbx, [rsi+ARRAY_OFF_INDEX]
  mul   rbx

  mov   rdi, [rsi+ARRAY_OFF_EL]
  add   rdi, rax

  mov   rcx, [rsi+ARRAY_OFF_EL_SIZE]
  mov   rsi, [rsp+0x8]
  rep   movsb

  mov   rdi, [rsp]
  inc   qword [rdi+ARRAY_OFF_INDEX]

  ; return the array
  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rdi, [rsp+0x20]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x28
  ret

; pops the last element of the array and returns it
; @param  rdi: pointer to the array structure
; @return rax: popped element
array_pop:
  cmp   rdi, 0
  jle   .error

  xor   rdx, rdx
  mov   rax, [rdi+ARRAY_OFF_EL_SIZE]
  mov   rbx, [rdi+ARRAY_OFF_INDEX]

  cmp   rbx, 0
  jle   .error

  dec   rbx
  mul   rbx

  mov   rsi, [rdi+ARRAY_OFF_EL]
  add   rsi, rax

  xor   rax, rax
  mov   rax, [rsi]
  mov   qword [rsi], 0

  dec   qword [rdi+ARRAY_OFF_INDEX]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the element at index i
; @param  rdi: pointer to the array struct
; @param  rsi: index of the element to get
; @return rax: return code
array_get:
  sub   rsp, 0x10

  ; *** STACK USAGE *** ;
  ; [rsp]     -> pointer to the array struct
  ; [rsp+0x8] -> index of the element to get

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  ; make sure the index required is not greater than current index
  mov   rax, [rdi+ARRAY_OFF_INDEX]
  cmp   rsi, rax
  jge   .error

  xor   rdx, rdx
  mov   rax, [rdi+ARRAY_OFF_EL_SIZE]
  mov   rbx, rsi
  mul   rbx

  mov   rsi, [rdi+ARRAY_OFF_EL]
  add   rsi, rax

  xor   rax, rax
  mov   rax, [rsi]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret


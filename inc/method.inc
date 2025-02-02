section .data
  method_struct:
    method_str    dq 0
    method_next   dq 0
  method_struct_end:

  METHOD_STRUCT_LEN equ method_struct_end - method_struct

  METHOD_OFF_STR    equ 0x0
  METHOD_OFF_NEXT   equ METHOD_OFF_STR + 0x8

section .text
; checks if method is a valid method
; @param  rdi: pointer to method
; @return rax: (bool) method is valid
is_valid_method:
  sub   rsp, 0x8

  mov   [rsp], rdi

  cmp   rdi, GET 
  jl    .is_not_valid

  cmp   rdi, CONNECT
  jg    .is_not_valid

  ; method should be 8-byte aligned
  mov   rdi, [rsp]
  mov   rsi, 8
  call  _align

  cmp   rax, [rsp]
  jne   .is_not_valid

  mov   rax, 1
  jmp   .return

.is_not_valid:
  mov   rax, 0
  
.return:
  add   rsp, 0x8
  ret

; checks if the method is allowed and returns its position in methods list
; @param  rdi: request method
; @return rax: method position in routes list
check_method:
  sub   rsp, 0x20

  mov   rsi, [rdi]
  mov   [rsp], rsi

  mov   rdi, rsi
  call  strlen

  cmp   rax, 0
  jle   .method_not_allowed

  mov   qword [rsp+0x8], 0
  mov   qword [rsp+0x10], rax

  ; look for the method inside methods list
  xor   rdi, rdi
  lea   rdi, [methods_list]
  mov   rsi, [rsp]
  call  strpos

  cmp   rax, 0
  jge   .method_exists
  
.method_not_allowed:
  mov   rax, FAILURE_CODE
  jmp   .return

.method_exists:
  ; store method's position onto the stack
  mov   qword [rsp+0x8], rax

  ; go to the method
  xor   rdi, rdi
  lea   rdi, [methods_list]
  add   rdi, qword [rsp+0x8]

  ; check if we are the beginning of the method
  dec   rdi
  mov   al, byte [rdi]
  cmp   al, SPACE
  jne   .method_not_allowed

  ; go to the end of the method
  inc   rdi
  add   rdi, qword [rsp+0x10]

  ; make sure the next byte is a space
  mov   al, byte [rdi]
  cmp   al, SPACE
  jne   .method_not_allowed

  ; check if the method is allowed
  xor   rdi, rdi
  lea   rdi, [not_allowed_methods]
  mov   rsi, [rsp]
  call  strpos

  cmp   rax, 0
  jge   .method_not_allowed

  ; return method's position
  mov   rax, qword [rsp+0x8]

.return:
  add   rsp, 0x20
  ret


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


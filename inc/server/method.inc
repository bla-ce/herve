section .bss
method_struct:
  method_str    resq 1
  method_next   resq 1
method_struct_end:

section .data
  ; methods
  align 8
  GET     db "GET", NULL_CHAR
  align 8
  POST    db "POST", NULL_CHAR
  align 8
  PUT     db "PUT", NULL_CHAR
  align 8
  DELETE  db "DELETE", NULL_CHAR
  align 8
  HEAD    db "HEAD", NULL_CHAR
  align 8
  OPTIONS db "OPTIONS", NULL_CHAR
  align 8
  PATCH   db "PATCH", NULL_CHAR
  align 8
  TRACE   db "TRACE", NULL_CHAR
  align 8
  CONNECT db "CONNECT", NULL_CHAR


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


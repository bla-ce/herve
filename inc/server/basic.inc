%include "base64.inc"

section .data
  BASIC_AUTH      db "Basic ", NULL_CHAR
  BASIC_AUTH_LEN  equ $ - BASIC_AUTH - 1 ; remove null char

section .text

; checks whether or not an authentication is done with Basic
; @param  rdi: pointer to authorization header
; @return rax: bool
is_basic_auth:
  sub   rsp, 0x8

  cmp   rdi, 0
  jle   .error
  
  mov   [rsp], rdi

  mov   rdi, [rsp]
  lea   rsi, [BASIC_AUTH]
  call  starts_with

  ; rax contains the boolean

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; reads and decodes basic authentication
; @param  rdi: pointer to authorization header
; @return rax: pointer to decoded string
read_basic_auth:
  sub   rsp, 0x8

  cmp   rdi, 0
  jle   .error

  mov   [rsp], rdi
 
  mov   rdi, [rsp]
  call  is_basic_auth
  cmp   rax, FALSE
  je    .error

  mov   rdi, [rsp]
  add   rdi, BASIC_AUTH_LEN   ; go after the basic
  call  base64_decode
  cmp   rax, 0
  jl    .error

  jmp   .return

.error:
  mov   rax, FAILURE_CODE
  
.return:
  add   rsp, 0x8
  ret


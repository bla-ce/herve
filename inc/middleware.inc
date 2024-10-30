section .bss
  middleware_chain resq MAX_MIDDLEWARES_COUNT
section .data
  MAX_MIDDLEWARES_COUNT equ 10
section .text

; TODO: add middleware to route
; adds a middleware to the end of the chain
; @param  rdi: pointer to server
; @param  rsi: middleware
; @return rax: return code
add_middleware:
  sub   rsp, 0x10

  mov   [rsp], rdi
  mov   qword [rsp+0x8], 0


.loop:
  lea   rdi, [middleware_chain]

  mov   rax, qword [rsp+0x8]
  mov   rbx, 0x8
  mul   rbx
  add   rdi, rax

  cmp   qword [rdi], 0
  je    .add

  inc   qword [rsp+0x8]

  cmp   qword [rsp+0x8], MAX_MIDDLEWARES_COUNT
  jg    .error

  jmp   .loop

.add:
  mov   rax, [rsp]
  mov   [rdi], rax

  mov   rax, SUCCESS_CODE
  jmp   .return
  
.error:
  mov    rax, FAILURE_CODE
  
.return:
  add   rsp, 0x10
  ret

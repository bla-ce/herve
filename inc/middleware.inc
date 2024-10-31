section .bss
section .data
  middleware_struct:
    middleware  dq 0
    next        dq 0 
  middleware_end:
section .text

; TODO: add middleware to route
; adds a middleware to the end of the chain
; @param  rdi: middleware
; @param  rsi: pointer to server
; @return rax: return code
add_middleware:
  sub   rsp, 0x10

  mov   [rsp], rdi

  lea   rdi, [middleware]

  ; check if first middleware is null
  cmp   qword [rdi], 0
  jne    .loop

  mov   rax, [rsp]
  mov   [rdi], rax
  jmp   .return

.loop:
  ; go to *next
  add   rdi, 0x8

  ; check if the address in rdi is null
  cmp   qword [rdi], 0
  je    .add

  ; rdi contains the address of next
  ; needs to contain the address pointer by next
  mov   rsi, rdi
  mov   rdi, [rsi]

  jmp   .loop

.add:
  ; save next pos onto the stack
  mov   [rsp+0x8], rdi

  ; mmap struct
  mov   rdi, middleware_end
  sub   rdi, middleware_struct
  call  mmap

  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp]

  ; rax contains address of new middleware struct
  ; rdi contains the address of next
  ; rsi contains the middleware

  ; mov middleware inside address of middleware struct
  mov   [rax], rsi

  ; mov middleware struct inside rdi
  mov   [rdi], rax

  mov   rax, SUCCESS_CODE
  jmp   .return
  
.error:
  mov    rax, FAILURE_CODE
  
.return:
  add   rsp, 0x10
  ret

; run the chain of middlewares
; @param  rdi: request
; @return rax: return code
run_middlewares:
  sub   rsp, 0x10

  mov   [rsp], rdi

  lea   rdi, [middleware_struct]
  cmp   qword [rdi], 0
  je    .end

.loop:
  ; rdi contains the address of middleware_struct
  mov   rax, [rdi]

  mov   [rsp+0x8], rdi  ; save address of middleware struct
  mov   rdi, [rsp]
  call  rax             ; call middleware

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]  ; load address of middleware struct

  add   rdi, 0x8        ; go to next
  cmp   qword [rdi], 0
  je    .end

  ; rdi contains the address of next
  ; needs to contains the address pointer by next
  mov   rsi, [rdi]
  mov   rdi, rsi

  jmp   .loop

.end:
  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret



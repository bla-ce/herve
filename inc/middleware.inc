section .bss
section .data

  middleware_struct:
    middleware_addr dq 0
    middleware_next dq 0 
  middleware_struct_end:

  MIDDLEWARE_STRUCT_LEN equ middleware_struct_end - middleware_struct

  MIDDLEWARE_OFF_ADDR equ 0x0
  MIDDLEWARE_OFF_NEXT equ MIDDLEWARE_OFF_ADDR + 0x8

section .text

; TODO: add middleware to route
; adds a middleware to the end of the chain
; @param  rdi: pointer to server
; @param  rsi: middleware
; @return rax: return code
add_middleware:
  sub   rsp, 0x18

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  call  get_server_middlewares

  cmp   rax, 0
  jl    .error

  ; check if first middleware is null
  mov   rdi, rax

  cmp   qword [rdi], 0
  je    .add

  mov   rdi, [rax]

.loop:
  ; go to *next
  add   rdi, MIDDLEWARE_OFF_NEXT

  ; check if the address in rdi is null
  cmp   qword [rdi], 0
  je    .add

  ; rdi contains the address of next
  ; needs to contain the address pointed by next
  mov   rsi, rdi
  mov   rdi, [rsi]

  jmp   .loop

.add:
  ; save next pos onto the stack
  mov   [rsp+0x10], rdi

  mov   rdi, MIDDLEWARE_STRUCT_LEN
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp+0x8]

  ; rax contains address of new middleware struct
  ; rdi contains the address of next
  ; rsi contains the middleware

  ; mov middleware inside address of middleware struct
  mov   [rax+MIDDLEWARE_OFF_ADDR], rsi
  mov   qword [rax+MIDDLEWARE_OFF_NEXT], 0

  ; mov middleware struct inside rdi
  mov   [rdi], rax

  mov   rax, SUCCESS_CODE
  jmp   .return
  
.error:
  mov    rax, FAILURE_CODE
  
.return:
  add   rsp, 0x18
  ret

; run the chain of middlewares
; @param  rdi: pointer to server struct
; @param  rsi: pointer to request struct
; @return rax: return code
run_middlewares:
  sub   rsp, 0x18

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  call  get_server_middlewares

  cmp   rax, 0
  jl    .error

  mov   rdi, [rax]

.loop:
  ; rdi contains the address of middleware_struct
  mov   rax, [rdi+MIDDLEWARE_OFF_ADDR]

  mov   [rsp+0x10], rdi
  mov   rdi, [rsp]
  call  rax

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x10]  ; load address of middleware struct

  add   rdi, MIDDLEWARE_OFF_NEXT
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
  add   rsp, 0x18
  ret



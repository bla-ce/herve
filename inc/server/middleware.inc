section .bss
middleware_struct:
  middleware_addr resq 1
  middleware_arg1 resq 1
  middleware_arg2 resq 1
  middleware_arg3 resq 1
middleware_struct_end:

section .data
  MIDDLEWARE_STRUCT_LEN equ middleware_struct_end - middleware_struct

  MIDDLEWARE_OFF_ADDR equ 0x0
  MIDDLEWARE_OFF_ARG1 equ MIDDLEWARE_OFF_ADDR + 0x8
  MIDDLEWARE_OFF_ARG2 equ MIDDLEWARE_OFF_ARG1 + 0x8
  MIDDLEWARE_OFF_ARG3 equ MIDDLEWARE_OFF_ARG2 + 0x8

  MIDDLEWARE_MAX_COUNT equ 15

section .text
; pops the last middleware from the end of the chain
; this function will mostly be used to pop current handler
; @param  rdi: pointer to the server
; @return rax: return code
pop_middleware:
  sub   rsp, 0x10

  ; *** STACK USAGE *** ;
  ; [rsp]     -> pointer to the server
  ; [rsp+0x8] -> pointer to the middleware chain
  
  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

  mov   rdi, [rsp]
  call  get_server_middlewares
  cmp   rax, 0
  jle   .error

  ; save middlewares array
  mov   [rsp+0x8], rax

  mov   rdi, rax
  cmp   qword [rdi], 0
  je    .loop_end

  xor   rbx, rbx

.loop:
  cmp   rbx, MIDDLEWARE_MAX_COUNT-1
  je    .pop
  jg    .loop_end

  mov   rcx, rbx
  inc   rcx

  ; check if next is null
  cmp   qword [rdi+8*rcx], 0
  je    .pop

  inc   rbx
  jmp   .loop

.pop:
  ; rdi contains a pointer to the middleware we are checking
  ; rsi is a pointer pointing to the address of the next middleware
  mov   qword [rdi+8*rbx], 0
.loop_end:

  mov   rax, SUCCESS_CODE
  jmp   .return
  
.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; TODO: add middleware to route
; adds a middleware to the end of the chain
; @param  rdi: pointer to server
; @param  rsi: middleware
; @param  rdx: arg1
; @param  rcx: arg2
; @param  r8:  arg3
; @return rax: return code
add_middleware:
  sub   rsp, 0x38

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the server struct
  ; [rsp+0x8]   -> middleware function
  ; [rsp+0x10]  -> arg 1 to be passed when calling the function
  ; [rsp+0x18]  -> arg 2 to be passed when calling the function
  ; [rsp+0x20]  -> arg 3 to be passed when calling the function
  ; [rsp+0x28]  -> pointer to the middlewares array
  ; [rsp+0x30]  -> pointer to middleware struct

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   [rsp+0x18], rcx
  mov   [rsp+0x20], r8

  mov   qword [rsp+0x30], 0

  mov   rdi, [rsp]
  call  get_server_middlewares
  cmp   rax, 0
  jl    .error

  mov   rdi, rax

  ; check if first middleware is null
  cmp   qword [rax], 0
  je    .add

  xor   rbx, rbx
.loop:
  ; get middleware
  mov   rax, [rdi+8*rbx]
  cmp   rax, 0
  je    .add

  inc   rbx
  cmp   rbx, MIDDLEWARE_MAX_COUNT
  jge   .error

  jmp   .loop

.add:
  lea   rax, [rdi+8*rbx]
  ; save next pos onto the stack
  mov   [rsp+0x28], rax

  mov   rdi, MIDDLEWARE_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x30], rax

  mov   rsi, [rsp+0x8]
  mov   rdx, [rsp+0x10]
  mov   rcx, [rsp+0x18]
  mov   r8, [rsp+0x20]

  ; mov middleware inside address of middleware struct
  mov   [rax+MIDDLEWARE_OFF_ADDR], rsi
  mov   [rax+MIDDLEWARE_OFF_ARG1], rdx  ; load first argument
  mov   [rax+MIDDLEWARE_OFF_ARG2], rcx  ; load second argument
  mov   [rax+MIDDLEWARE_OFF_ARG3], r8  ; load third argument

  ; mov middleware struct inside rdi
  mov   rdi, [rsp+0x28]
  mov   [rdi], rax

  mov   rax, SUCCESS_CODE
  jmp   .return
  
.error:
  mov   rax, [rsp+0x30]
  test  rax, rax
  jz    .no_free

  mov   rdi, rax
  call  free

.no_free:
  mov    rax, FAILURE_CODE
  
.return:
  add   rsp, 0x38
  ret

; run the chain of middlewares
; @param  rdi: pointer to server struct
; @param  rsi: pointer to context struct
; @return rax: return code
run_middlewares:
  sub   rsp, 0x18

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the server struct
  ; [rsp+0x8]   -> pointer to the context struct
  ; [rsp+0x10]  -> pointer to the middlewares array

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  call  get_server_middlewares
  cmp   rax, 0
  jl    .error
  je    .end

  mov   [rsp+0x10], rax

.loop:
  mov   rax, [rsp+0x8]
  mov   rbx, qword [rax+CTX_OFF_INDEX]
  cmp   rbx, MIDDLEWARE_MAX_COUNT
  jge   .end
  
  mov   rax, [rsp+0x10]
  mov   rdi, [rax+8*rbx]

  cmp   rdi, 0
  je    .end

  ; rdi contains the address of middleware_struct
  mov   rax, [rdi+MIDDLEWARE_OFF_ADDR]

  mov   rsi, [rdi+MIDDLEWARE_OFF_ARG1]  ; load first argument
  mov   rdx, [rdi+MIDDLEWARE_OFF_ARG2]  ; load second argument
  mov   rcx, [rdi+MIDDLEWARE_OFF_ARG3]  ; load third argument
  mov   rdi, [rsp+0x8]
  call  rax

  ; check if the contxt has been aborted
  mov   rdi, [rsp+0x8]
  call  is_ctx_aborted
  
  cmp   rax, TRUE
  je    .return

  mov   rax, [rsp+0x8]
  inc   qword [rax+CTX_OFF_INDEX]

  jmp   .loop

.end:
  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret



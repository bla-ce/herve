section .bss
middleware_struct:
  middleware_addr     resq 1
  middleware_arg1     resq 1
  middleware_arg2     resq 1
  middleware_next     resq 1
  middleware_post_req resq 1
middleware_struct_end:

section .data
  MIDDLEWARE_STRUCT_LEN equ middleware_struct_end - middleware_struct

  MIDDLEWARE_OFF_ADDR     equ 0x0
  MIDDLEWARE_OFF_ARG1     equ MIDDLEWARE_OFF_ADDR + 0x8
  MIDDLEWARE_OFF_ARG2     equ MIDDLEWARE_OFF_ARG1 + 0x8
  MIDDLEWARE_OFF_NEXT     equ MIDDLEWARE_OFF_ARG2 + 0x8
  MIDDLEWARE_OFF_POST_REQ equ MIDDLEWARE_OFF_NEXT + 0x8

section .text
; adds a middleware to the end of the linked list of the group
; @param  rdi: pointer to server
; @param  rsi: middleware
; @param  rdx: pointer to the group or 0 if root
; @param  rcx: arg1
; @param  r8:  arg2
; @param  r9:  post request middlewware (bool)
; @return rax: return code
add_middleware:
  sub   rsp, 0x40

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the server struct
  ; [rsp+0x8]   -> middleware function
  ; [rsp+0x10]  -> pointer to the group or 0 if root
  ; [rsp+0x18]  -> arg 1 to be passed when calling the function
  ; [rsp+0x20]  -> arg 2 to be passed when calling the function
  ; [rsp+0x28]  -> post request middleware (bool)
  ; [rsp+0x30]  -> pointer to go through the list of middlewares
  ; [rsp+0x38]  -> pointer to middleware struct

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   [rsp+0x18], rcx
  mov   [rsp+0x20], r8
  mov   [rsp+0x28], r9
  mov   qword [rsp+0x38], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  cmp   rdx, 0
  jl    .error

  ; get the root group if the value is 0
  mov   rdi, [rsp+0x10]
  test  rdi, rdi
  jnz   .no_root_group

  mov   rdi, [rsp]
  call  get_server_root_group
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax
  mov   rdi, rax

.no_root_group:
  add   rdi, GRP_STRUCT_OFF_MIDDLEWARES

  cmp   qword [rdi], 0
  je    .add

  ; check if first middleware is null
  mov   rsi, [rdi]
  mov   rdi, rsi

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
  mov   [rsp+0x30], rdi

  mov   rdi, MIDDLEWARE_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x38], rax

  mov   rsi, [rsp+0x8]
  mov   rdx, [rsp+0x18]
  mov   rcx, [rsp+0x20]
  mov   r9, [rsp+0x28]

  ; mov middleware inside address of middleware struct
  mov   [rax+MIDDLEWARE_OFF_ADDR], rsi
  mov   [rax+MIDDLEWARE_OFF_ARG1], rdx  ; load first argument
  mov   [rax+MIDDLEWARE_OFF_ARG2], rcx  ; load second argument
  mov   qword [rax+MIDDLEWARE_OFF_POST_REQ], r9
  mov   qword [rax+MIDDLEWARE_OFF_NEXT], 0

  ; mov middleware struct inside rdi
  mov   rdi, [rsp+0x30]
  mov   [rdi], rax

  mov   rax, SUCCESS_CODE
  jmp   .return
  
.error:
  mov   rdi, [rsp+0x38]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov    rax, FAILURE_CODE
  
.return:
  add   rsp, 0x40
  ret

; run the chain of middlewares
; @param  rdi: pointer to group struct
; @param  rsi: pointer to context struct
; @param  rdx: run post middlewares (bool)
; @return rax: return code
run_middlewares:
  sub   rsp, 0x20

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the group struct
  ; [rsp+0x8]   -> pointer to the context struct
  ; [rsp+0x10]  -> run post middlewares (bool)
  ; [rsp+0x18]  -> pointer to middleware address

  cmp   rdi, 0
  jl    .error
  je    .end

  cmp   rsi, 0
  jle   .error

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx

  call  get_group_middlewares
  cmp   rax, 0
  jl    .error
  je    .end

  mov   rdi, rax

.loop:
  mov   [rsp+0x18], rdi
  mov   rax, qword [rdi+MIDDLEWARE_OFF_POST_REQ]
  mov   rbx, qword [rsp+0x10]
  cmp   rax, rbx
  jne   .skip

  ; rdi contains the address of middleware_struct
  mov   rax, [rdi+MIDDLEWARE_OFF_ADDR]
  mov   [rsp+0x18], rdi
  mov   rsi, [rdi+MIDDLEWARE_OFF_ARG1]  ; load first argument
  mov   rdx, [rdi+MIDDLEWARE_OFF_ARG2]  ; load first argument
  mov   rdi, [rsp+0x8]
  call  rax
  cmp   rax, 0
  jl    .error

  ; check if the contxt has been aborted
  mov   rdi, [rsp+0x8]
  call  is_ctx_aborted
  cmp   rax, TRUE
  je    .return

.skip:
  mov   rdi, [rsp+0x18]  ; load address of middleware struct

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
  add   rsp, 0x20
  ret



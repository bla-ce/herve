section .bss
middleware_t:
  .addr     resq 1
  .arg1     resq 1
  .arg2     resq 1
  .arg3     resq 1
  .post_req resq 1
middleware_t_end:

section .data

MIDDLEWARE_STRUCT_LEN equ middleware_t_end - middleware_t

MIDDLEWARE_OFF_ADDR     equ 0x0
MIDDLEWARE_OFF_ARG1     equ MIDDLEWARE_OFF_ADDR + 0x8
MIDDLEWARE_OFF_ARG2     equ MIDDLEWARE_OFF_ARG1 + 0x8
MIDDLEWARE_OFF_ARG3     equ MIDDLEWARE_OFF_ARG2 + 0x8
MIDDLEWARE_OFF_POST_REQ equ MIDDLEWARE_OFF_ARG3 + 0x8

section .text
; creates a middleware structure - dev is responsible of freeing
; the structure
; @param  rdi: pointer to the function
; @param  rsi: arg1
; @param  rdx: arg2
; @param  rcx: arg3
; @param  r9: post request (boolean)
; @return rax: pointer to the middleware
middleware_create:
  sub   rsp, 0x28

  ; STACK USAGE
  ; [rsp]       -> pointer to the function
  ; [rsp+0x8]   -> arg1
  ; [rsp+0x10]  -> arg2
  ; [rsp+0x18]  -> arg3
  ; [rsp+0x20]  -> post request middleware

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   [rsp+0x18], rcx
  mov   [rsp+0x20], r9

  cmp   rdi, 0
  jle   .error

  ; malloc structure
  mov   rdi, MIDDLEWARE_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  mov   rdx, [rsp+0x10]
  mov   rcx, [rsp+0x18]
  mov   r9, [rsp+0x20]

  ; add arguments
  mov   qword [rax+MIDDLEWARE_OFF_ADDR], rdi
  mov   qword [rax+MIDDLEWARE_OFF_ARG1], rsi
  mov   qword [rax+MIDDLEWARE_OFF_ARG2], rdx
  mov   qword [rax+MIDDLEWARE_OFF_ARG3], rcx
  mov   qword [rax+MIDDLEWARE_OFF_POST_REQ], r9

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x28
  ret

; adds a middleware to the end of the linked list of the group
; @param  rdi: pointer to server
; @param  rsi: pointer to the group or 0 if root
; @param  rdx: pointer to a middleware struct
; @return rax: return code
middleware_add:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> pointer to the server struct
  ; [rsp+0x8]   -> pointer to the group or 0 if root
  ; [rsp+0x10]  -> pointer to a middleware struct

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx

  test  rdi, rdi
  jz    .error

  test  rdx, rdx
  jz    .error

  ; get the root group if the value is 0
  mov   rdi, [rsp+0x8]
  test  rdi, rdi
  jnz   .no_root_group

  mov   rdi, [rsp]
  call  server_get_root_group
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

.no_root_group:
  mov   rdi, [rsp+0x8]
  lea   rdi, [rdi+GRP_OFF_MIDDLEWARES]
  mov   rsi, [rsp+0x10]
  call  linked_list_insert_at_first
  cmp   byte [_linked_list_errno], TRUE
  je    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov    rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; run the chain of middlewares
; @param  rdi: pointer to group struct
; @param  rsi: pointer to context struct
; @param  rdx: run post middlewares (bool)
; @return rax: return code
middleware_run_chain:
  push  rbx

  sub   rsp, 0x28

  ; STACK USAGE
  ; [rsp]       -> pointer to the group struct
  ; [rsp+0x8]   -> pointer to the context struct
  ; [rsp+0x10]  -> run post middlewares (bool)
  ; [rsp+0x18]  -> middleware linked list
  ; [rsp+0x20]  -> pointer to temp middleware

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx

  test  rdi, rdi
  jz    .end

  test  rsi, rsi
  jz    .error

  lea   rax, [rdi+GRP_OFF_MIDDLEWARES]
  mov   [rsp+0x18], rax

  mov   qword [rax], 0
  je    .end

.loop:
  mov   rdi, [rsp+0x18]
  call  linked_list_get_node_data
  cmp   byte [_linked_list_errno], TRUE
  je    .error

  mov   [rsp+0x20], rax

  mov   rdi, [rax+MIDDLEWARE_OFF_POST_REQ]
  mov   rbx, qword [rsp+0x10]
  cmp   rdi, rbx
  jne   .skip

  ; rdi contains the address of middleware_struct
  mov   rdi, [rsp+0x8]
  mov   rsi, [rax+MIDDLEWARE_OFF_ARG1]  ; load first argument
  mov   rdx, [rax+MIDDLEWARE_OFF_ARG2]  ; load second argument
  mov   rcx, [rax+MIDDLEWARE_OFF_ARG3]  ; load third argument
  mov   rbx, [rax+MIDDLEWARE_OFF_ADDR]
  call  rbx

  ; check if the context has been aborted
  mov   rdi, [rsp+0x8]
  call  ctx_is_aborted
  cmp   rax, TRUE
  je    .end

.skip:
  mov   rdi, [rsp+0x18]
  call  linked_list_get_node_next
  cmp   byte [_linked_list_errno], TRUE
  je    .error

  test  rax, rax
  jz    .end

  mov   [rsp+0x18], rax

  jmp   .loop

.end:
  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  ; abort request
  mov   rdi, [rsp+0x8]
  call  ctx_abort

  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x28

  pop   rbx

  ret

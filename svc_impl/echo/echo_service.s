section .data

echo_svc_t:
  .id         dq 0  ; set by the central server
  .name       dq 0  ; set by the caller
  .status     dq 0  ; set by the central server
  .port       dq 0  ; set by the central server
  .type       dq SVC_TYPES.ECHO
  .register   dq echo_svc_register
  .unregister dq echo_svc_unregister
  .start      dq echo_svc_start
  .stop       dq echo_svc_stop
  .next       dq 0  ; set by the central server
echo_svc_t_end:

echo_svc_msg:
  .register db "Registering new echo server...", NULL_CHAR


echo_url db "/echo", NULL_CHAR

section .text
; returns the content of the body
; @param  rdi: pointer to the context struct
; @return rax: return code
echo_handler:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp] -> pointer to the context struct

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

  ; get request
  mov   rdi, [rsp]
  call  get_ctx_request
  cmp   rax, 0
  jl    .error

  ; get request body
  mov   rdi, rax
  call  get_request_body
  cmp   rax, 0
  jl    .error

  ; send body
  mov   rdi, [rsp]
  mov   rsi, OK
  mov   rdx, rax
  call  send_string
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; registers a new echo service
; @param  rdi: pointer to the context struct
; @return rax: return code
echo_svc_register:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> pointer to the context struct
  ; [rsp+0x8] -> pointer to the server struct

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

  ; get server from context struct
  mov   rdi, [rsp]
  call  get_ctx_server
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; add endpoints to the server
  mov   rdi, [rsp+0x8]
  mov   rsi, POST
  mov   rdx, echo_url
  mov   rcx, echo_handler
  mov   r8, NO_ARG
  mov   r9, NO_ARG
  call  add_route
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

echo_svc_unregister:
  ret

echo_svc_start:
  ret

echo_svc_stop:
  ret

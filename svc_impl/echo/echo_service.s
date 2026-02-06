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
; @param  rsi: pointer to the server
; @return rax: return code
echo_svc_register:
  sub   rsp, 0x8

  ; get server from context
  call  get_ctx_server
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, POST
  mov   rdx, echo_url
  mov   rcx, echo_handler
  mov		r8, NO_ARG
  call  add_route
  cmp   rax, 0
  jl    .error

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

echo_svc_unregister:
  ret

echo_svc_start:
  ret

echo_svc_stop:
  ret

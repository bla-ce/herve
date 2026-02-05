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

section .text

; registers a new echo service
; @param  rdi: pointer to the context struct
; @param  rsi: pointer to the service struct
; @return rax: return code
echo_svc_register:
  ; get server from context
  call  get_ctx_server
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  server_get_logger
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, echo_svc_msg.register
  call  log_infoln
  cmp   rax, 0
  jl    .error

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

echo_svc_unregister:
  ret

echo_svc_start:
  ret

echo_svc_stop:
  ret

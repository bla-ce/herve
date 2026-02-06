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
; registers a new echo service
; @param  rdi: pointer to the context struct
; @param  rsi: pointer to the server
; @return rax: return code
echo_svc_register:
  ret

echo_svc_unregister:
  ret

echo_svc_start:
  ret

echo_svc_stop:
  ret

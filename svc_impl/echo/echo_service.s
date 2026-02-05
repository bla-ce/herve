section .data

echo_svc_t:
  .id         db 0  ; set by the central server
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

section .text

; registers a new echo service
; @param  rdi: pointer to the service struct
echo_svc_register:
  ret

echo_svc_unregister:
  ret

echo_svc_start:
  ret

echo_svc_stop:
  ret

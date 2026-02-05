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

; initialises a new echo service
; @param  rdi: pointer to the service struct
; @return rax: return code
echo_svc_init:
  cmp   rdi, 0
  jle    .error

  mov   qword [rdi+SERVICE_OFF_TYPE], SVC_TYPES.ECHO
  mov   qword [rdi+SERVICE_OFF_REGISTER], echo_svc_register
  mov   qword [rdi+SERVICE_OFF_UNREGISTER], echo_svc_unregister
  mov   qword [rdi+SERVICE_OFF_START], echo_svc_start
  mov   qword [rdi+SERVICE_OFF_STOP], echo_svc_stop

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

echo_svc_register:
  ret

echo_svc_unregister:
  ret

echo_svc_start:
  ret

echo_svc_stop:
  ret

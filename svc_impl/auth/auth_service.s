section .data

auth_svc_t:
  .id         dq 0  ; set by the central server
  .name       dq 0  ; set by the caller
  .status     dq 0  ; set by the central server
  .type       dq SVC_TYPES.AUTH
  .register   dq auth_svc_register
  .unregister dq auth_svc_unregister
  .start      dq auth_svc_start
  .stop       dq auth_svc_stop
  .group      dq 0  ; set by the central server
  .next       dq 0  ; set by the central server
auth_svc_t_end:

auth_svc_msg:
  .register db "Registering new auth server...", NULL_CHAR

auth_url  db "/auth", NULL_CHAR

section .text
; registers a new auth service
; @param  rdi: pointer to the context struct
; @param  rsi: pointer to the service struct
; @return rax: return code
auth_svc_register:
  sub   rsp, 0x20

  ; STACK USAGE
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> pointer to the service struct
  ; [rsp+0x10]  -> pointer to the server struct
  ; [rsp+0x18]  -> pointer to group

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret

auth_svc_unregister:
  ret

auth_svc_start:
  ret

auth_svc_stop:
  ret

section .data

ping_svc_t:
  .id         dq 0  ; set by the central server
  .name       dq 0  ; set by the caller
  .status     dq 0  ; set by the central server
  .type       dq SVC_TYPES.PING
  .register   dq ping_svc_register
  .unregister dq ping_svc_unregister
  .start      dq ping_svc_start
  .stop       dq ping_svc_stop
  .group      dq 0  ; set by the central server
  .next       dq 0  ; set by the central server
ping_svc_t_end:

ping_svc_msg:
  .register db "Registering new ping server...", NULL_CHAR

ping_url  db "/ping", NULL_CHAR

pong_msg db "pong", LINE_FEED, NULL_CHAR

section .text
; returns the content of the body
; @param  rdi: pointer to the context struct
; @return rax: return code
ping_handler:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp] -> pointer to the context struct

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

  ; send body
  mov   rdi, [rsp]
  mov   rsi, OK
  mov   rdx, pong_msg
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

; registers a new ping service
; @param  rdi: pointer to the context struct
; @param  rsi: pointer to the service struct
; @return rax: return code
ping_svc_register:
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

  ; get server from context struct
  mov   rdi, [rsp]
  call  ctx_get_server
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  mov   rdi, [rsp+0x8]
  call  service_get_group
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; add endpoints to the server
  mov   rdi, [rsp+0x10]
  mov   rsi, GET
  mov   rdx, ping_url
  mov   rcx, ping_handler
  mov   r8, [rsp+0x18]
  mov   r9, NO_ARG
  call  route_add
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret

ping_svc_unregister:
  ret

ping_svc_start:
  ret

ping_svc_stop:
  ret

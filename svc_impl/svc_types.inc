%include "echo_service.s"
%include "ping_service.s"

section .data

SVC_TYPES:
  .ECHO db "echo", NULL_CHAR
  .PING db "ping", NULL_CHAR

section .text

; initialises service function pointers based on type
; @param  rdi: pointer to service struct
; @param  rsi: type string
; @return rax: return code
svc_init_by_type:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> pointer to the service struct
  ; [rsp+0x8] -> pointer to the type of the service

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; get the service implementation
  mov   rdi, [rsp+0x8]
  call  svc_get_from_str_type
  cmp   rax, 0
  jl    .error

  ; get service to be created
  mov   rdi, [rsp]

  ; init type and function pointers
  mov   rsi, qword [rax+SERVICE_OFF_TYPE]
  mov   qword [rdi+SERVICE_OFF_TYPE], rsi

  mov   rsi, [rax+SERVICE_OFF_REGISTER]
  mov   qword [rdi+SERVICE_OFF_REGISTER], rsi

  mov   rsi, [rax+SERVICE_OFF_UNREGISTER]
  mov   qword [rdi+SERVICE_OFF_UNREGISTER], rsi

  mov   rsi, [rax+SERVICE_OFF_START]
  mov   qword [rdi+SERVICE_OFF_START], rsi

  mov   rsi, [rax+SERVICE_OFF_STOP]
  mov   qword [rdi+SERVICE_OFF_STOP], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; converts type string to enum value
; @param  rdi: type string
; @return rax: type enum value
svc_get_from_str_type:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp]   -> pointer to the type string

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

.test_echo:
  mov   rdi, [rsp]
  mov   rsi, SVC_TYPES.ECHO
  call  strcmp
  cmp   rax, 0
  jl    .error
  cmp   rax, TRUE
  jne   .test_ping

  mov   rax, echo_svc_t

  jmp   .return

.test_ping:
  mov   rdi, [rsp]
  mov   rsi, SVC_TYPES.PING
  call  strcmp
  cmp   rax, 0
  jl    .error
  cmp   rax, TRUE
  jne   .error

  mov   rax, ping_svc_t

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

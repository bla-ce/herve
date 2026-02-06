%include "echo_service.s"

section .data

SVC_STR:
  .ECHO db "echo", NULL_CHAR

SVC_TYPES:
  .ECHO   equ 0

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

  mov   rdi, [rsp+0x8]
  call  svc_type_from_str
  cmp   rax, 0
  jl    .error

  cmp   rax, SVC_TYPES.ECHO
  jne   .error

  mov   rax, echo_svc_t
  jmp   .init_svc

.init_svc:
  ; get service
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

; converts type enum value to string
; @param rdi: type enum value
; @return rax: type string
svc_type_to_str:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp]   -> type enum value

  mov   [rsp], rdi

  cmp   rdi, 0
  jl    .error

  cmp   rdi, SVC_TYPES.ECHO
  jne   .error ; unknown service

  mov   rax, SVC_STR.ECHO

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; converts type string to enum value
; @param  rdi: type string
; @return rax: type enum value
svc_type_from_str:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp]   -> pointer to the type string

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

  mov   rdi, [rsp]
  mov   rsi, SVC_STR.ECHO
  call  strcmp
  cmp   rax, 0
  jl    .error
  cmp   rax, TRUE
  jne   .error  ; unknown service

  mov   rax, SVC_TYPES.ECHO

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

%include "echo_service.s"

section .data

SVC_STR:
  .ECHO db "echo", NULL_CHAR

SVC_TYPES:
  .ECHO   equ 0

section .text

; initializes service function pointers based on type
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

  mov   rdi, [rsp]
  call  echo_svc_register
  cmp   rax, 0
  jl    .error

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

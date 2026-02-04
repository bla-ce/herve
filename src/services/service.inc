section .bss
service_t:
  .name   resq 1
  .next   resq 1
service_t_end:

section .data

SERVICE_STRUCT_LEN equ service_t_end - service_t

SERVICE_OFF_NAME  equ 0x0
SERVICE_OFF_NEXT  equ SERVICE_OFF_NAME + 0x8

service_key:
  .name db "name", NULL_CHAR

hv_services dq 0   ; linked list of services

section .text

; registers a new service and adds it to the service linked list
; of herve
; @param  rdi: context struct
; @return rax: return code
service_register:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> pointer to the service struct
  ; [rsp+0x10]  -> pointer to the form data (hash table)

  mov   [rsp], rdi

  cmp   rdi, 0
  jle    .error

  ; malloc new service
  mov   rdi, SERVICE_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; get request struct
  mov   rdi, [rsp]
  call  get_ctx_request
  cmp   rax, 0
  jl    .error

  ; get form data
  mov   rdi, rax
  call  parse_form_data
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; get name of the service
  mov   rdi, [rsp+0x10]
  mov   rsi, service_key.name
  call  ht_get
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  service_set_name
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  xor   rsi, rsi
  call  service_set_next
  cmp   rax, 0
  jl    .error

  mov   rdi, hv_services
  mov   rsi, [rsp+0x8]
  mov   rdx, SERVICE_OFF_NEXT
  call  linked_list_add_entry
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rdi, [rsp+0x8]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; unregisters an existing service and removes it from the service
; linked list of herve
; @param  rdi: context struct
; @return rax: return code
service_unregister:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp]   -> pointer to the context struct

  mov   [rsp], rdi

  cmp   rdi, 0
  jle    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; returns the name of the service
; @param  rdi: pointer to the service struct
; @return rax: name of the service
service_get_name:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+SERVICE_OFF_NAME]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the name of the service
; @param  rdi: pointer to the service struct
; @param  rsi: name of the service
; @return rax: return code
service_set_name:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   [rdi+SERVICE_OFF_NAME], rsi

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the next pointer of the service
; @param  rdi: pointer to the service struct
; @return rax: pointer to the next service in the linked list
service_get_next:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+SERVICE_OFF_NEXT]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the next pointer of the service
; @param  rdi: pointer to the service struct
; @param  rsi: pointer to the service struct
; @return rax: return code
service_set_next:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   [rdi+SERVICE_OFF_NEXT], rsi

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

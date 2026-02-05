section .bss
service_t:
  .id         resq 1
  .name       resq 1
  .status     resq 1
  .port       resq 1
  .type       resq 1
  .register   resq 1  ; ptr to the func registering the service
  .unregister resq 1  ; ptr to the func unregistering the service
  .start      resq 1  ; ptr to the func starting the service
  .stop       resq 1  ; ptr to the func stopping the service
  .next       resq 1
service_t_end:

section .data

SERVICE_STRUCT_LEN equ service_t_end - service_t

SERVICE_OFF_NAME        equ 0x0
SERVICE_OFF_STATUS      equ SERVICE_OFF_NAME + 0x8
SERVICE_OFF_PORT        equ SERVICE_OFF_STATUS + 0x8
SERVICE_OFF_TYPE        equ SERVICE_OFF_PORT + 0x8
SERVICE_OFF_REGISTER    equ SERVICE_OFF_TYPE + 0x8
SERVICE_OFF_UNREGISTER  equ SERVICE_OFF_REGISTER + 0x8
SERVICE_OFF_START       equ SERVICE_OFF_UNREGISTER + 0x8
SERVICE_OFF_STOP        equ SERVICE_OFF_START + 0x8
SERVICE_OFF_NEXT        equ SERVICE_OFF_STOP + 0x8

service_key:
  .name   db "name", NULL_CHAR
  .status db "status", NULL_CHAR
  .port   db "port", NULL_CHAR
  .type   db "type", NULL_CHAR

service_status:
  .NOT_STARTED  db "NOT STARTED", NULL_CHAR
  .RUNNING      db "RUNNING", NULL_CHAR
  .PAUSED       db "PAUSED", NULL_CHAR
  .STOPPED      db "STOPPED", NULL_CHAR

; endpoints
service_url:
  .list       db "/services", NULL_CHAR
  .register   db "/services/register", NULL_CHAR
  .unregister db "/services/unregister", NULL_CHAR

hv_services dq 0   ; linked list of services

MAX_SERVICES equ 32

service_count db 0

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
  mov   qword [rsp+0x8], 0

  cmp   rdi, 0
  jle    .internal_server_error

  mov   al, byte [service_count]
  inc   al

  cmp   al, MAX_SERVICES
  jge   .internal_server_error

  ; malloc new service
  mov   rdi, SERVICE_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .internal_server_error

  mov   [rsp+0x8], rax

  ; get request struct
  mov   rdi, [rsp]
  call  get_ctx_request
  cmp   rax, 0
  jl    .internal_server_error

  ; get form data
  mov   rdi, rax
  call  parse_form_data
  cmp   rax, 0
  jl    .internal_server_error

  mov   [rsp+0x10], rax

  ; get name of the service
  mov   rdi, [rsp+0x10]
  mov   rsi, service_key.name
  call  ht_get
  cmp   rax, 0
  jl    .internal_server_error

  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  service_set_name
  cmp   rax, 0
  jl    .internal_server_error

  ; get port of the service
  mov   rdi, [rsp+0x10]
  mov   rsi, service_key.port
  call  ht_get
  cmp   rax, 0
  jl    .internal_server_error

  mov   rdi, rax
  call  stoi

  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  service_set_port
  cmp   rax, 0
  jl    .internal_server_error

  mov   rdi, [rsp+0x8]
  mov   rsi, service_status.NOT_STARTED
  call  service_set_status
  cmp   rax, 0
  jl    .internal_server_error

  mov   rdi, [rsp+0x8]
  xor   rsi, rsi
  call  service_set_next
  cmp   rax, 0
  jl    .internal_server_error

  mov   rdi, [rsp+0x8]
  mov   qword [rdi+SERVICE_OFF_REGISTER], 0
  mov   qword [rdi+SERVICE_OFF_UNREGISTER], 0
  mov   qword [rdi+SERVICE_OFF_START], 0
  mov   qword [rdi+SERVICE_OFF_STOP], 0

  mov   rdi, hv_services
  mov   rsi, [rsp+0x8]
  mov   rdx, SERVICE_OFF_NEXT
  call  linked_list_add_entry
  cmp   rax, 0
  jl    .internal_server_error

  inc   byte [service_count]

  ; return 201 -> TODO: return json
  mov   rdi, [rsp]
  mov   rsi, CREATED
  call  send_no_content
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.internal_server_error:
  ; TODO: return json
  mov   rdi, [rsp]
  mov   rsi, INTERNAL_SERVER_ERROR
  call  send_no_content

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

; returns a list of available services
; @param  rdi: context struct
service_list:
  sub   rsp, 0x20

  ; STACK USAGE
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> pointer to json array
  ; [rsp+0x10]  -> dynamic pointer to go through the service ll
  ; [rsp+0x18]  -> service json

  mov   [rsp], rdi
  mov   qword [rsp+0x8], 0

  cmp   rdi, 0
  jle    .error

  call  json_array_create
  cmp   rax, 0
  jl    .internal_server_error

  mov   [rsp+0x8], rax

  mov   rdi, [hv_services]
  mov   [rsp+0x10], rdi
  cmp   rdi, 0
  je    .loop_end

.loop:
  call  json_create
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  mov   rdi, [rsp+0x10]
  call  service_get_name
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x18]
  mov   rsi, service_key.name
  mov   rdx, rax
  call  json_insert_string
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  mov   rdi, [rsp+0x10]
  call  service_get_status
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x18]
  mov   rsi, service_key.status
  mov   rdx, rax
  call  json_insert_string
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  mov   rdi, [rsp+0x10]
  call  service_get_port
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x18]
  mov   rsi, service_key.port
  mov   rdx, rax
  call  json_insert_integer
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  mov   rdi, [rsp+0x18]
  call  json_end
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x18]
  call  json_array_insert_object
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, [rsp+0x18]
  call  json_free
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x10]
  call  service_get_next
  cmp   rax, 0
  jl    .error
  je    .loop_end

  mov   [rsp+0x10], rax

  jmp   .loop

.loop_end:
  mov   rdi, [rsp+0x8]
  call  json_array_end
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, [rsp]
  mov   rsi, OK
  mov   rdx, [rsp+0x8]
  call  send_JSON
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  call  json_array_free
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.internal_server_error:
  ; TODO: return json
  mov   rdi, [rsp]
  mov   rsi, INTERNAL_SERVER_ERROR
  call  send_no_content

.error:
  mov   rdi, [rsp+0x18]
  test  rdi, rdi
  jz    .free_json_array

  call  json_free

.free_json_array:
  mov   rdi, [rsp+0x8]
  test  rdi, rdi
  jz    .no_free

  call  json_array_free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
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

; returns the status of the service
; @param  rdi: pointer to the service struct
; @return rax: status of the service
service_get_status:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+SERVICE_OFF_STATUS]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the status of the service
; @param  rdi: pointer to the service struct
; @param  rsi: status of the service
; @return rax: return code
service_set_status:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   [rdi+SERVICE_OFF_STATUS], rsi

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the port of the service
; @param  rdi: pointer to the service struct
; @return rax: port of the service
service_get_port:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+SERVICE_OFF_PORT]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the port of the service
; @param  rdi: pointer to the service struct
; @param  rsi: port of the service
; @return rax: return code
service_set_port:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   [rdi+SERVICE_OFF_PORT], rsi

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
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

%include "svc_types.inc"

section .bss
service_t:
  .uuid       resq 1
  .name       resq 1
  .status     resq 1
  .type       resq 1
  .register   resq 1  ; ptr to the func registering the service
  .unregister resq 1  ; ptr to the func unregistering the service
  .start      resq 1  ; ptr to the func starting the service
  .stop       resq 1  ; ptr to the func stopping the service
  .group      resq 1  ; ptr to the server group with all the routes
  .next       resq 1
service_t_end:

section .data

SERVICE_STRUCT_LEN equ service_t_end - service_t

SERVICE_OFF_UUID        equ 0x0
SERVICE_OFF_NAME        equ SERVICE_OFF_UUID + 0x8
SERVICE_OFF_STATUS      equ SERVICE_OFF_NAME + 0x8
SERVICE_OFF_TYPE        equ SERVICE_OFF_STATUS + 0x8
SERVICE_OFF_REGISTER    equ SERVICE_OFF_TYPE + 0x8
SERVICE_OFF_UNREGISTER  equ SERVICE_OFF_REGISTER + 0x8
SERVICE_OFF_START       equ SERVICE_OFF_UNREGISTER + 0x8
SERVICE_OFF_STOP        equ SERVICE_OFF_START + 0x8
SERVICE_OFF_GRP         equ SERVICE_OFF_STOP + 0x8
SERVICE_OFF_NEXT        equ SERVICE_OFF_GRP + 0x8

service_key:
  .uuid   db "uuid", NULL_CHAR
  .name   db "name", NULL_CHAR
  .status db "status", NULL_CHAR
  .type   db "type", NULL_CHAR

service_status:
  .REGISTERED   db "registered", NULL_CHAR
  .RUNNING      db "running", NULL_CHAR
  .STOPPED      db "stopped", NULL_CHAR

; endpoints
service_endpoint:
  .root         db "/", NULL_CHAR
  .service_root db "/services", NULL_CHAR
  .register     db "/services/register", NULL_CHAR
  .unregister   db "/services/:uuid/unregister", NULL_CHAR
  .start        db "/services/:uuid/start", NULL_CHAR
  .stop         db "/services/:uuid/stop", NULL_CHAR

hv_services dq 0   ; linked list of services

MAX_SERVICES equ 32

service_msg:
  .registered   db "The service has been successfully registered.", NULL_CHAR
  .unregistered db "The service has been successfully unregistered.", NULL_CHAR
  .started      db "The service has successfully started.", NULL_CHAR
  .stopped      db "The service has successfully stopped.", NULL_CHAR

  .stop_to_unregister db "The service should be stopped before being unregistered", NULL_CHAR

service_err:
  .unknown_type     db "This service type does not exist.", NULL_CHAR
  .unknown_service  db "The service does not exist.", NULL_CHAR
  .generic          db "An error occurred, please try again later", NULL_CHAR

service_count   dq 0
service_curr_id dq 0

section .text

; registers a new service and adds it to the service linked list
; of herve
; @param  rdi: context struct
; @return rax: return code
service_register:
  sub   rsp, 0x20

  ; STACK USAGE
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> pointer to the service struct
  ; [rsp+0x10]  -> pointer to the group
  ; [rsp+0x18]  -> pointer to the service to json object

  mov   [rsp], rdi
  mov   qword [rsp+0x18], 0

  cmp   rdi, 0
  jle    .internal_server_error

  mov   rax, qword [service_count]
  inc   rax

  cmp   rax, MAX_SERVICES
  jge   .internal_server_error

  mov   rdi, [rsp]
  call  service_init
  cmp   rax, 0
  jl    .internal_server_error

  mov   [rsp+0x8], rax

  mov   rdi, [rsp+0x8]
  call  service_get_type
  cmp   rax, 0
  jl    .internal_server_error

  ; from the type, infer the service
  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  svc_init_by_type
  cmp   rax, 0
  jl    .not_found

  ; get server struct
  mov   rdi, [rsp]
  call  ctx_get_server
  cmp   rax, 0
  jl    .internal_server_error

  ; create server group from service
  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  service_set_group
  cmp   rax, 0
  jl    .internal_server_error

  mov   [rsp+0x10], rax

  ; create default route for the service
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  call  service_init_routes
  cmp   rax, 0
  jl    .internal_server_error

  ; register service
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  mov   rax, [rsi+SERVICE_OFF_REGISTER]
  call  rax
  cmp   rax, 0
  jl    .internal_server_error

  ; deactivate the group of routes
  mov   rdi, [rsp+0x10]
  call  group_deactivate
  cmp   rax, 0
  jl    .internal_server_error

  mov   rdi, hv_services
  mov   rsi, [rsp+0x8]
  mov   rdx, SERVICE_OFF_NEXT
  call  linked_list_add_entry
  cmp   rax, 0
  jl    .internal_server_error

  inc   qword [service_count]
  inc   qword [service_curr_id]

  ; service to json
  mov   rdi, [rsp+0x8]
  call  service_to_json
  cmp   rax, 0
  jl    .internal_server_error

  mov   [rsp+0x18], rax

  mov   rdi, [rsp]
  mov   rsi, CREATED
  mov   rdx, service_msg.registered
  mov   rcx, [rsp+0x18]
  mov   r8, TRUE
  call  service_return_json
  cmp   rax, 0
  jl    .error

  ; free service to json
  mov   rdi, [rsp+0x18]
  call  json_free
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.not_found:
  mov   rsi, NOT_FOUND
  mov   rdx, service_err.unknown_type

  jmp   .send_json

.internal_server_error:
  mov   rsi, INTERNAL_SERVER_ERROR
  mov   rdx, service_err.generic

.send_json:
  mov   rdi, [rsp]
  mov   rcx, NO_ARG
  mov   r8, FALSE
  call  service_return_json

.error:
  mov   rdi, [rsp+0x8]
  test  rdi, rdi
  jz    .free_json_response

  call  free

  ; remove service from ll
  mov   rdi, hv_services
  mov   rsi, [rsp+0x8]
  mov   rdx, SERVICE_OFF_NEXT
  call  linked_list_remove_entry

.free_json_response:
  mov   rdi, [rsp+0x18]
  test  rdi, rdi
  jz    .no_free

  call  json_free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret

; initialises a service before registering it
; @param  rdi: pointer to the context struct
; @return rax: pointer to the service struct
service_init:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]     -> pointer to the context struct
  ; [rsp+0x8] -> pointer to the service struct
  ; [rsp+0x10]  -> pointer to the form data (hash table)

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

  ; malloc new service
  mov   rdi, SERVICE_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; get request struct
  mov   rdi, [rsp]
  call  ctx_get_request
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

  ; generate uuid
  call  uuid_v4
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  service_set_uuid
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, service_status.REGISTERED
  call  service_set_status
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  xor   rsi, rsi
  call  service_set_next
  cmp   rax, 0
  jl    .error

  ; get the type of the service
  mov   rdi, [rsp+0x10]
  mov   rsi, service_key.type
  call  ht_get
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  service_set_type
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x8]

  jmp   .return

.error:
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
  mov   rdi, [rsp+0x10]
  call  service_to_json
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
  mov   rdx, NO_ARG
  mov   rcx, [rsp+0x8]
  mov   r8, TRUE
  call  service_return_json
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
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> pointer to the service struct

  mov   [rsp], rdi

  cmp   rdi, 0
  jle    .error

  ; get the request struct
  mov   rdi, [rsp]
  call  ctx_get_request
  cmp   rax, 0
  jl    .internal_server_error

  ; get the dynamic parameter from the request
  mov   rdi, rax
  mov   rsi, service_key.uuid
  call  route_get_dynamic_param
  cmp   rax, 0
  jl    .internal_server_error

  ; convert str to int
  mov   rdi, rax
  call  stoi
  cmp   rax, 0  ; should not be below zero
  jl    .internal_server_error

  ; get the service by its uuid
  mov   rdi, rax
  call  service_get_by_uuid
  cmp   rax, 0
  jl    .not_found

  mov   [rsp+0x8], rax

  ; check if the service is stopped to be unregistered
  mov   rdi, [rsp+0x8]
  call  service_get_status
  cmp   rax, 0
  jl    .internal_server_error

  cmp   rax, service_status.STOPPED
  je    .svc_is_stopped
  cmp   rax, service_status.REGISTERED
  je    .svc_is_stopped

  mov   rdi, [rsp]
  mov   rsi, BAD_REQUEST
  mov   rdx, service_msg.stop_to_unregister
  mov   rcx, NO_ARG
  mov   r8, FALSE
  call  service_return_json

  jmp   .error

.svc_is_stopped:
  ; remove service from the linked list
  mov   rdi, hv_services
  mov   rsi, [rsp+0x8]
  mov   rdx, SERVICE_OFF_NEXT
  call  linked_list_remove_entry
  cmp   rax, 0
  jl    .error

  dec   qword [service_count]

  ; free the service
  mov   rdi, [rsp+0x8]
  call  service_free
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, OK
  mov   rdx, service_msg.unregistered
  mov   rcx, NO_ARG
  mov   r8, TRUE
  call  service_return_json
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.not_found:
  mov   rsi, NOT_FOUND
  mov   rdx, service_err.unknown_service

  jmp   .send_json

.internal_server_error:
  mov   rsi, INTERNAL_SERVER_ERROR
  mov   rdx, service_err.generic

.send_json:
  mov   rdi, [rsp]
  mov   rcx, NO_ARG
  mov   r8, FALSE
  call  service_return_json

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; returns a service json response
; @param  rdi: pointer to the context struct
; @param  rsi: status code
; @param  rdx: pointer to the message
; @param  rcx: pointer pointer to the json object
; @param  r8: success
; @return rax: return code
service_return_json:
  sub   rsp, 0x30

  ; STACK USAGE
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> status code
  ; [rsp+0x10]  -> pointer to the message
  ; [rsp+0x18]  -> pointer to the json object
  ; [rsp+0x20]  -> success (boolean)
  ; [rsp+0x28]  -> pointer to the json response object

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   [rsp+0x18], rcx
  mov   [rsp+0x20], r8
  mov   qword [rsp+0x28], 0

  mov   rdi, [rsp+0x20]
  mov   rsi, [rsp+0x10]
  mov   rdx, [rsp+0x18]
  call  create_json_response
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x28], rax

  ; return 201 -> TODO: return json
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  mov   rdx, rax
  call  send_json
  cmp   rax, 0
  jl    .error

  ; free json response
  mov   rdi, [rsp+0x28]
  call  json_free
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rdi, [rsp+0x28]
  test  rdi, rdi
  jz    .no_free

  call  json_free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x30
  ret

; converts a service struct to JSON
; user is responsible for freeing the json
; @param  rdi: pointer to the service struct
; @return rax: pointer to json
service_to_json:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> pointer to the service struct
  ; [rsp+0x8] -> pointer to the json

  cmp   rdi, 0
  jle   .error

  mov   [rsp], rdi
  mov   qword [rsp+0x8], 0

  call  json_create
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, [rsp]
  call  service_get_uuid
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, service_key.uuid
  mov   rdx, rax
  call  json_insert_string
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, [rsp]
  call  service_get_name
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, service_key.name
  mov   rdx, rax
  call  json_insert_string
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, [rsp]
  call  service_get_status
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, service_key.status
  mov   rdx, rax
  call  json_insert_string
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, [rsp]
  call  service_get_type
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, service_key.type
  mov   rdx, rax
  call  json_insert_string
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, [rsp+0x8]
  call  json_end
  cmp   rax, 0
  jl    .error

  jmp   .return

.error:
  mov   rdi, [rsp+0x8]
  test  rdi, rdi
  jz    .no_free

  call  json_free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; initialises the start and stop route for the service
; @param  rdi: pointer to the context struct
; @param  rsi: pointer to the service struct
service_init_routes:
  sub   rsp, 0x20

  ; STACK USAGE
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> pointer to the service struct
  ; [rsp+0x10]  -> pointer to the server struct
  ; [rsp+0x18]  -> pointer to group prefix

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

  ; get start function
  mov   rsi, [rsp+0x8]
  mov   rax, [rsi+SERVICE_OFF_START]

  ; add endpoints to the server
  mov   rdi, [rsp+0x10]
  mov   rsi, GET
  mov   rdx, service_endpoint.start
  mov   rcx, rax
  mov   r8, [rsp+0x18]
  mov   r9, [rsp+0x8]
  call  route_add
  cmp   rax, 0
  jl    .error

  ; get stop function
  mov   rsi, [rsp+0x8]
  mov   rax, [rsi+SERVICE_OFF_STOP]

  ; add endpoints to the server
  mov   rdi, [rsp+0x10]
  mov   rsi, GET
  mov   rdx, service_endpoint.stop
  mov   rcx, rax
  mov   r8, [rsp+0x18]
  mov   r9, [rsp+0x8]
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

; starts a service if it exists
; @param  rdi: pointer to the context struct
service_start:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> uuid of the service
  ; [rsp+0x10]  -> pointer to the service

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

  ; get the request struct
  mov   rdi, [rsp]
  call  ctx_get_request
  cmp   rax, 0
  jl    .internal_server_error

  ; get the dynamic parameter from the request
  mov   rdi, rax
  mov   rsi, service_key.uuid
  call  route_get_dynamic_param
  cmp   rax, 0
  jl    .internal_server_error

  mov   [rsp+0x8], rax

  mov   rdi, [rsp+0x8]
  call  service_get_by_uuid
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; activate the group of the service
  mov   rdi, [rsp+0x10]
  call  service_get_group
  cmp   rax, 0
  jl    .internal_server_error

  mov   rdi, rax
  call  group_activate
  cmp   rax, 0
  jl    .internal_server_error

  ; set the service status
  mov   rdi, [rsp+0x10]
  mov   rsi, service_status.RUNNING
  call  service_set_status
  cmp   rax, 0
  jl    .error

  ; call the start function of the service
  mov   rax, [rsp+0x10]
  mov   rdi, [rax+SERVICE_OFF_START]
  call  rdi
  cmp   rax, 0
  jl    .internal_server_error

  mov   rdi, [rsp]
  mov   rsi, OK
  mov   rdx, service_msg.started
  mov   rcx, NO_ARG
  mov   r8, TRUE
  call  service_return_json
  cmp   rax, 0
  jl    .error

  jmp   .return

.not_found:
  mov   rsi, NOT_FOUND
  mov   rdx, service_err.unknown_service

  jmp   .send_json

.internal_server_error:
  mov   rsi, INTERNAL_SERVER_ERROR
  mov   rdx, service_err.generic

.send_json:
  mov   rdi, [rsp]
  mov   rcx, NO_ARG
  mov   r8, FALSE
  call  service_return_json

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; stops a service if it exists
; @param  rdi: pointer to the context struct
; @return rax: return code
service_stop:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> uuid of the service
  ; [rsp+0x10]  -> pointer to the service

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

  ; get the request struct
  mov   rdi, [rsp]
  call  ctx_get_request
  cmp   rax, 0
  jl    .internal_server_error

  ; get the dynamic parameter from the request
  mov   rdi, rax
  call  get_request_dynamic_param
  cmp   rax, 0
  jl    .internal_server_error

  ; get the uuid
  mov   rdi, rax
  mov   rsi, service_key.uuid
  call  ht_get
  cmp   rax, 0
  jl    .internal_server_error

  mov   [rsp+0x8], rax

  mov   rdi, rax
  call  service_get_by_uuid
  cmp   rax, 0
  jl    .not_found

  mov   [rsp+0x10], rax

  ; activate the group of the service
  call  service_get_group
  cmp   rax, 0
  jl    .internal_server_error

  mov   rdi, rax
  call  group_deactivate
  cmp   rax, 0
  jl    .internal_server_error

  ; set the service status
  mov   rdi, [rsp+0x10]
  mov   rsi, service_status.STOPPED
  call  service_set_status
  cmp   rax, 0
  jl    .error

  ; call the start function of the service
  mov   rax, [rsp+0x10]
  mov   rdi, [rax+SERVICE_OFF_STOP]
  call  rdi
  cmp   rax, 0
  jl    .internal_server_error

  mov   rdi, [rsp]
  mov   rsi, OK
  mov   rdx, service_msg.stopped
  mov   rcx, NO_ARG
  mov   r8, TRUE
  call  service_return_json
  cmp   rax, 0
  jl    .error

  jmp   .return

.not_found:
  mov   rsi, NOT_FOUND
  mov   rdx, service_err.unknown_service

  jmp   .send_json

.internal_server_error:
  mov   rsi, INTERNAL_SERVER_ERROR
  mov   rdx, service_err.generic

.send_json:
  mov   rdi, [rsp]
  mov   rcx, NO_ARG
  mov   r8, FALSE
  call  service_return_json

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; returns a service by its uuid
; @param  rdi: uuid of the service
; @return rax: pointer to the service struct
service_get_by_uuid:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> id of the service
  ; [rsp+0x8] -> pointer to current service

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

  ; get the services
  mov   rdi, [hv_services]
  cmp   rdi, 0
  je    .error

  mov   [rsp+0x8], rdi

.loop:
  mov   rdi, [rsp+0x8]

  ; get the uuid of the service
  mov   rax, [rdi+SERVICE_OFF_UUID]

  mov   rdi, rax
  mov   rsi, [rsp]
  call  strcmp
  cmp   rax, 0
  jl    .error
  cmp   rax, TRUE
  je    .loop_end

  ; go to next
  mov   rdi, [rsp+0x8]
  mov   rsi, [rdi+SERVICE_OFF_NEXT]
  cmp   rsi, 0
  je    .error

  mov   [rsp+0x8], rsi

  jmp   .loop

.loop_end:
  mov   rax, [rsp+0x8]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; returns the type of the service
; @param  rdi: pointer to the service struct
; @return rax: type of the service
service_get_type:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+SERVICE_OFF_TYPE]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the type of the service
; @param  rdi: pointer to the service struct
; @param  rsi: type of the service
; @return rax: return code
service_set_type:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   [rdi+SERVICE_OFF_TYPE], rsi

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
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

; returns the uuid of the service
; @param  rdi: pointer to the service struct
; @return rax: uuid of the service
service_get_uuid:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+SERVICE_OFF_UUID]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the uuid of the service
; @param  rdi: pointer to the service struct
; @param  rsi: uuid of the service
; @return rax: return code
service_set_uuid:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   [rdi+SERVICE_OFF_UUID], rsi

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the server group of the service
; @param  rdi: pointer to the service struct
; @return rax: pointer to the server group of the service
service_get_group:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+SERVICE_OFF_GRP]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the server group of the service from its struct and add it
; to the server
; @param  rdi: pointer to the service struct
; @param  rsi: pointer to the server struct
; @return rax: return code
service_set_group:
  sub   rsp, 0x20
  sub   rsp, TO_STRING_MAX_STR_SIZE

  ; STACK USAGE
  ; [rsp]       -> pointer to the service struct
  ; [rsp+0x8]   -> pointer to the server struct
  ; [rsp+0x10]  -> pointer to the group prefix
  ; [rsp+0x18]  -> pointer to the group struct
  ; [rsp+0x20]  -> pointer to uuid

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x10], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; get uuid of the service
  mov   rdi, [rsp]
  call  service_get_uuid
  cmp   rax, 0
  jl    .error

  ; create group endpoint
  mov   rdi, service_endpoint.root
  mov   rsi, rax
  call  strcat
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; create group
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x10]
  mov   rdx, FALSE
  call  group_create
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  mov   rdi, [rsp]
  mov   [rdi+SERVICE_OFF_GRP], rax

  mov   rdi, [rsp+0x10]
  call  free
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x18]

  jmp   .return

.error:
  mov   rdi, [rsp+0x10]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, TO_STRING_MAX_STR_SIZE
  add   rsp, 0x20
  ret

; frees the service and associated resources
; @param  rdi: pointer to the service
; @return rax: return code
service_free:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp] -> pointer to the service

  mov  [rsp], rdi

  cmp   rdi, 0
  jle   .error

  ; free the uuid
  mov   rdi, [rsp]
  call  service_get_uuid
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  free
  cmp   rax, 0
  jl    .error

  ; free the group
  mov   rdi, [rsp]
  call  service_get_group
  cmp   rax, 0
  jl    .error

  ; group free removes the group from the server
  mov   rdi, rax
  call  group_free
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  call  free
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

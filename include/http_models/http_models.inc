%include "model_json.inc"

section .data

http_model_err:
  .not_found            db " model: instance not found", NULL_CHAR
  .internal_server      db " model: An unexpected error occured", NULL_CHAR
  .unprocessable_entity db " model: unable to process this content", NULL_CHAR

http_model_msg:
  .model_deleted db " model: instance has been deleted", NULL_CHAR

data_key    db "data", NULL_CHAR

http_model_route_root db "/", NULL_CHAR
http_model_route_id   db "/:id", NULL_CHAR

section .text
; returns all instances of a model
; @param  rdi: pointer to the context struct
; @param  rsi: pointer to the model struct
; @return rax: return code
http_model_read_many:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> pointer to the model struct
  ; [rsp+0x10]  -> pointer to the json object

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x10], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   rdi, [rsp+0x8]
  call  model_instances_to_json
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; send response
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  mov   rdx, OK
  mov   rcx, NO_ARG
  mov   r8, [rsp+0x10]
  mov   r9, TRUE
  call  model_http_response
  ; no need to check the error here

  ; free array
  mov   rdi, [rsp+0x10]
  call  json_array_free
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rdi, [rsp+0x10]
  test  rdi, rdi
  jz    .no_free

  call  json_array_free

.no_free:
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  mov   rdx, INTERNAL_SERVER_ERROR
  mov   rcx, http_model_err.internal_server
  mov		r8, NO_ARG
  mov   r9, FALSE
  call  model_http_response

  ; TODO: log error?

  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; returns an instance of the model from its id
; @param  rdi: pointer to the context struct
; @param  rsi: pointer to the model struct
; @return rax: return code
http_model_read_single:
  sub   rsp, 0x28

  ; STACK USAGE
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> pointer to the model struct
  ; [rsp+0x10]  -> pointer to the instance
  ; [rsp+0x18]  -> json object of the instance
  ; [rsp+0x20]  -> pointer to the response message

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x10], 0
  mov   qword [rsp+0x18], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; get context request
  mov   rdi, [rsp]
  call  ctx_get_request
  cmp   rax, 0
  jle   .error

  ; get id from param
  mov   rdi, rax
  mov   rsi, id_field_name
  call  route_get_dynamic_param
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  stoi
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  model_get_instance_by_id
  cmp   rax, 0
  jl    .error
  je    .not_found

  mov   [rsp+0x10], rax

  ; add instance to json
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x10]
  call  model_instance_to_json
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  mov   rdx, OK
  mov		rcx, NO_ARG
  mov   r8, [rsp+0x18]
  mov   r9, TRUE
  call  model_http_response

  mov   rdi, [rsp+0x18]
  call  json_free
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.not_found:
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  mov   rdx, NOT_FOUND
  mov   rcx, http_model_err.not_found
  mov		r8, NO_ARG
  mov   r9, FALSE
  call  model_http_response

  ; TODO: log?

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rdi, [rsp+0x20]
  test  rdi, rdi
  jz    .free_json

  call  free

.free_json:
  mov   rdi, [rsp+0x18]
  test  rdi, rdi
  jz    .no_free

  call  json_free

.no_free:
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  mov   rdx, INTERNAL_SERVER_ERROR
  mov   rcx, http_model_err.internal_server
  mov		r8, NO_ARG
  mov   r9, FALSE
  call  model_http_response

  ; TODO: log?

  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x28
  ret

; update an instance of a model
; @param  rdi: pointer to the context struct
; @param  rsi: pointer to the model struct
; @return rax: return code
http_model_put:
  sub   rsp, 0x38

  ; STACK USAGE
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> pointer to the model struct
  ; [rsp+0x10]  -> pointer to the instance of the model
  ; [rsp+0x18]  -> pointer to form data (hash table)
  ; [rsp+0x20]  -> pointer to fields linked list of the model
  ; [rsp+0x28]  -> name of the current field
  ; [rsp+0x30]  -> pointer to the json of the instance

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x18], 0
  mov   qword [rsp+0x30], 0

  cmp   rdi, 0
  jl    .internal_server_error

  cmp   rsi, 0
  jl    .internal_server_error

  ; get request
  mov   rdi, [rsp]
  call  ctx_get_request
  cmp   rax, 0
  jl    .internal_server_error

  ; get id from param
  mov   rdi, rax
  mov   rsi, id_field_name
  call  route_get_dynamic_param
  cmp   rax, 0
  jl    .internal_server_error

  mov   rdi, rax
  call  stoi
  cmp   rax, 0
  jl    .create_instance

  ; get the instance of the model from the id
  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  model_get_instance_by_id
  cmp   rax, 0
  jl    .internal_server_error
  je    .create_instance

  mov   [rsp+0x10], rax

  jmp   .populate_fields

.create_instance:
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  call  http_model_post
  cmp   rax, 0
  jl    .internal_server_error

  jmp   .return

.populate_fields:
  ; get fields from the body
  mov   rdi, [rsp]
  call  ctx_get_request
  cmp   rax, 0
  jl    .internal_server_error

  ; get form data
  mov   rdi, rax
  call  parse_form_data
  cmp   rax, 0
  jl    .internal_server_error

  mov   [rsp+0x18], rax

  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x10]
  mov   rdx, [rsp+0x18]
  mov   rcx, TRUE
  call  model_instance_update
  cmp   rax, 0
  jl    .internal_server_error

  ; add instance to json
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x10]
  call  model_instance_to_json
  cmp   rax, 0
  jl    .internal_server_error

  mov   [rsp+0x30], rax

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  mov   rdx, OK
  mov		rcx, NO_ARG
  mov   r8, [rsp+0x30]
  mov   r9, TRUE
  call  model_http_response

  mov   rdi, [rsp+0x18]
  call  ht_free
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x30]
  call  json_free
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.internal_server_error:
  mov   rdx, INTERNAL_SERVER_ERROR
  mov   rcx, http_model_err.internal_server
  jmp   .send_response

.unprocessable_entity:
  ; TODO: specify why
  mov   rdx, UNPROCESSABLE_ENTITY
  mov   rcx, http_model_err.unprocessable_entity

.send_response:
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  mov		r8, NO_ARG
  mov   r9, FALSE
  call  model_http_response

.error:
  mov   rdi, [rsp+0x18]
  test  rdi, rdi
  jz    .free_json

  call  ht_free

.free_json:
  mov   rdi, [rsp+0x30]
  test  rdi, rdi
  jz    .no_free

  call  json_free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x38
  ret

; patch an instance of a model
; @param  rdi: pointer to the context struct
; @param  rsi: pointer to the model struct
; @return rax: return code
http_model_patch:
  sub   rsp, 0x28

  ; STACK USAGE
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> pointer to the model struct
  ; [rsp+0x10]  -> pointer to the instance of the model
  ; [rsp+0x18]  -> form data in a hash table
  ; [rsp+0x20]  -> json of the instance

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x18], 0
  mov   qword [rsp+0x20], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; get context request
  mov   rdi, [rsp]
  call  ctx_get_request
  cmp   rax, 0
  jle   .error

  ; get id from param
  mov   rdi, rax
  mov   rsi, id_field_name
  call  route_get_dynamic_param
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  stoi
  cmp   rax, 0
  jl    .error

  ; get the instance of the model from the id
  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  model_get_instance_by_id
  cmp   rax, 0
  jl    .error
  je    .not_found

  mov   [rsp+0x10], rax

  ; get fields from the body
  mov   rdi, [rsp]
  call  ctx_get_request
  cmp   rax, 0
  jl    .error

  ; get form data
  mov   rdi, rax
  call  parse_form_data
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x10]
  mov   rdx, [rsp+0x18]
  mov   rcx, TRUE
  call  model_instance_update
  cmp   rax, 0
  jl    .internal_server_error

  ; add instance to json
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x10]
  call  model_instance_to_json
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  mov   rdx, OK
  mov		rcx, NO_ARG
  mov   r8, [rsp+0x20]
  mov   r9, TRUE
  call  model_http_response

  mov   rdi, [rsp+0x20]
  call  json_free

  mov   rdi, [rsp+0x18]
  call  ht_free

  mov   rax, SUCCESS_CODE
  jmp   .return

.internal_server_error:
  mov   rdx, INTERNAL_SERVER_ERROR
  mov   rcx, http_model_err.internal_server
  jmp   .send_response

.unprocessable_entity:
  ; TODO: specify why
  mov   rdx, UNPROCESSABLE_ENTITY
  mov   rcx, http_model_err.unprocessable_entity
  jmp   .send_response

.not_found:
  mov   rdx, NOT_FOUND
  mov   rcx, http_model_err.not_found

.send_response:
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  mov		r8, NO_ARG
  mov   r9, FALSE
  call  model_http_response

.error:
  mov   rdi, [rsp+0x18]
  test  rdi, rdi
  jz    .free_json

  call  ht_free

.free_json:
  mov   rdi, [rsp+0x20]
  test  rdi, rdi
  jz    .no_free

  call  json_free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x28
  ret

; TODO: accept json, requires the parser to be implemented
; TODO: check for variable types
; creates an instance of the model from the form data
; @param  rdi: pointer to the context struct
; @param  rsi: pointer to the model struct
; @return rax: return code
http_model_post:
  sub   rsp, 0x48

  ; STACK USAGE
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> pointer to the model struct
  ; [rsp+0x10]  -> pointer to the form data (hash table)
  ; [rsp+0x18]  -> number of fields
  ; [rsp+0x20]  -> array of ht keys
  ; [rsp+0x28]  -> pointer to the new instance
  ; [rsp+0x30]  -> dynamic pointer
  ; [rsp+0x38]  -> pointer to the current field struct
  ; [rsp+0x40]  -> json of the instance

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x28], 0
  mov   qword [rsp+0x40], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; get request struct
  mov   rdi, [rsp]
  call  ctx_get_request
  cmp   rax, 0
  jl    .internal_server_error

  ; get form data
  mov   rdi, rax
  call  parse_form_data
  cmp   rax, 0
  jl    .internal_server_error

  mov   [rsp+0x10], rax

  mov   rdi, [rsp+0x10]
  call  ht_get_length
  cmp   rax, 0
  jl    .internal_server_error

  mov   [rsp+0x18], rax

  ; get list of fields
  mov   rdi, [rsp+0x10]
  call  ht_get_keys
  cmp   rax, 0
  jl    .internal_server_error

  mov   [rsp+0x20], rax

  ; create new instance of the model
  mov   rdi, [rsp+0x8]
  call  model_instance_create
  cmp   rax, 0
  jl    .internal_server_error

  mov   [rsp+0x28], rax

.loop:
  dec   qword [rsp+0x18]
  cmp   qword [rsp+0x18], 0
  jl    .loop_end

  mov   rsi, [rsp+0x20]

  mov   r9, qword [rsp+0x18]

  ; go to the key
  mov   rax, [rsi+r9*8]

  mov   [rsp+0x30], rax

  ; check if the key exists
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x30]
  call  model_field_exist
  cmp   rax, 0
  jl    .internal_server_error
  je    .unprocessable_entity

  ; save field structure on the stack
  mov   [rsp+0x38], rax

  ; get the value
  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp+0x30]
  call  ht_get
  cmp   rax, 0
  jle   .unprocessable_entity

  ; convert the value to the right type
  mov   rdi, [rsp+0x38]
  mov   rsi, rax
  call  field_convert_str_to_value
  cmp   rax, 0
  jl    .internal_server_error

  ; set field value
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x28]
  mov   rdx, [rsp+0x30]
  mov   rcx, rax
  call  model_instance_set_value
  cmp   rax, 0
  jl    .internal_server_error

  jmp   .loop

.loop_end:
  ; save instance
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x28]
  mov   rdx, TRUE
  call  model_instance_save
  cmp   rax, 0
  jl    .unprocessable_entity

  ; add instance to json
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x28]
  call  model_instance_to_json
  cmp   rax, 0
  jl    .internal_server_error

  mov   [rsp+0x40], rax

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  mov   rdx, CREATED
  mov		rcx, NO_ARG
  mov   r8, [rsp+0x40]
  mov   r9, TRUE
  call  model_http_response

  ; free list of keys
  mov   rdi, [rsp+0x20]
  call  free
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.internal_server_error:
  mov   rdx, INTERNAL_SERVER_ERROR
  mov   rcx, http_model_err.internal_server

  jmp   .send_response

.unprocessable_entity:
  ; TODO: specify why
  mov   rdx, UNPROCESSABLE_ENTITY
  mov   rcx, http_model_err.unprocessable_entity

.send_response:
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  mov		r8, NO_ARG
  mov   r9, FALSE
  call  model_http_response

.error:
  mov   rdi, [rsp+0x28]
  test  rdi, rdi
  jz    .free_json_instance

  call  free

.free_json_instance:
  mov   rdi, [rsp+0x40]
  test  rdi, rdi
  jz    .no_free

  call  json_free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x48
  ret

; removes an instance of the model from its id
; @param  rdi: pointer to the context struct
; @param  rsi: pointer to the model struct
; @return rax: return code
http_model_delete:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> pointer to the model struct
  ; [rsp+0x10]  -> pointer to the response message

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x10], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; get context request
  mov   rdi, [rsp]
  call  ctx_get_request
  cmp   rax, 0
  jle   .error

  ; get id from param
  mov   rdi, rax
  mov   rsi, id_field_name
  call  route_get_dynamic_param
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  stoi
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  model_remove_instance_by_id
  cmp   rax, 0
  jl    .not_found

  mov   rdi, [rsp+0x8]
  call  model_get_name
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, http_model_msg.model_deleted
  call  strcat
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  mov   rdx, OK
  mov   rcx, http_model_msg.model_deleted
  mov		r8, NO_ARG
  mov   r9, TRUE
  call  model_http_response

  mov   rax, SUCCESS_CODE
  jmp   .return

.not_found:
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  mov   rdx, NOT_FOUND
  mov   rcx, http_model_err.not_found
  mov   r8, NO_ARG
  mov   r9, FALSE
  call  model_http_response

  ; return not found
  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rdi, [rsp+0x10]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  mov   rdx, INTERNAL_SERVER_ERROR
  mov   rcx, http_model_err.internal_server
  mov		r8, NO_ARG
  mov   r9, FALSE
  call  model_http_response

  ; TODO: log?

  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; sends json response to client with optional message and object
; @param  rdi: pointer to the context struct
; @param  rsi: pointer to the model struct
; @param  rdx: status code
; @param  rcx: pointer to the message
  ; message will be prefixed by the name of the model
; @param  r8:  pointer to the json object
; @param  r9:  success
; @return rax: return code
model_http_response:
  sub   rsp, 0x40

  ; STACK USAGE
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> pointer to the model struct
  ; [rsp+0x10]  -> status code
  ; [rsp+0x18]  -> pointer to the message
  ; [rsp+0x20]  -> pointer to the json object
  ; [rsp+0x28]  -> success (boolean)
  ; [rsp+0x30]  -> pointer to the concatenated string
  ; [rsp+0x38]  -> pointer to the response object

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   [rsp+0x18], rcx
  mov   [rsp+0x20], r8
  mov   [rsp+0x28], r9
  mov   qword [rsp+0x30], 0
  mov   qword [rsp+0x38], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  cmp   rdx, 0
  jle   .error

  cmp   rcx, 0
  jl    .error

  ; get model name
  mov   rdi, [rsp+0x8]
  call  model_get_name
  cmp   rax, 0
  jl    .skip_message

  cmp   qword [rsp+0x18], 0
  je    .skip_message

  ; create string -> prefix + message
  mov   rdi, rax
  mov   rsi, [rsp+0x18]
  call  strcat
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x30], rax

.skip_message:
  mov   rdi, [rsp+0x28]
  mov   rsi, [rsp+0x30]
  mov   rdx, [rsp+0x20]
  call  create_json_response
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x38], rax

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x10]
  mov   rdx, [rsp+0x38]
  call  send_JSON
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x38]
  call  json_free
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x30]
  test  rdi, rdi
  jz    .skip_free_msg

  call  free

.skip_free_msg:
  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  ; free resources
  mov   rdi, [rsp+0x38]
  test  rdi, rdi
  jz    .free_concat_str

  call  json_free

.free_concat_str:
  mov   rdi, [rsp+0x30]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x40
  ret

; creates a list of routes from a model
; @param  rdi: pointer to the server struct
; @param  rsi: pointer to the model struct
; @return rax: return code
add_model_routes:
  sub   rsp, 0x20
  sub   rsp, BUFSIZ

  ; STACK USAGE
  ; [rsp]       -> pointer to the server struct
  ; [rsp+0x8]   -> pointer to the model struct
  ; [rsp+0x10]  -> pointer to the model name
  ; [rsp+0x18]  -> pointer to the model group
  ; [rsp+0x20]  -> group url

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  cmp   rdi, 0
  jl    .error

  cmp   rsi, 0
  jl    .error

  ; get the name of the model
  mov   rdi, [rsp+0x8]
  call  model_get_name
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; create group url
  lea   rdi, [rsp+0x20]
  mov   rax, SLASH
  stosb

  mov   rsi, [rsp+0x10]
  call  strcpy
  cmp   rax, 0
  jl    .error

  ; create group
  mov   rdi, [rsp]
  lea   rsi, [rsp+0x20]
  mov   rdx, FALSE
  call  group_create
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; create routes
  ; GET -> http_model_read_many
  mov   rdi, [rsp]
  mov   rsi, GET
  mov   rdx, http_model_route_root
  mov   rcx, http_model_read_many
  mov   r8, [rsp+0x18]
  mov   r9, [rsp+0x8]
  call  route_add
  cmp   rax, 0
  jl    .error

  ; GET -> http_model_read_single
  mov   rdi, [rsp]
  mov   rsi, GET
  mov   rdx, http_model_route_id
  mov   rcx, http_model_read_single
  mov   r8, [rsp+0x18]
  mov   r9, [rsp+0x8]
  call  route_add
  cmp   rax, 0
  jl    .error

  ; POST -> http_model_post
  mov   rdi, [rsp]
  mov   rsi, POST
  mov   rdx, http_model_route_root
  mov   rcx, http_model_post
  mov   r8, [rsp+0x18]
  mov   r9, [rsp+0x8]
  call  route_add
  cmp   rax, 0
  jl    .error

  ; PUT -> http_model_update
  mov   rdi, [rsp]
  mov   rsi, PUT
  mov   rdx, http_model_route_id
  mov   rcx, http_model_put
  mov   r8, [rsp+0x18]
  mov   r9, [rsp+0x8]
  call  route_add
  cmp   rax, 0
  jl    .error

  ; PATCH -> http_model_patch
  mov   rdi, [rsp]
  mov   rsi, PATCH
  mov   rdx, http_model_route_id
  mov   rcx, http_model_patch
  mov   r8, [rsp+0x18]
  mov   r9, [rsp+0x8]
  call  route_add
  cmp   rax, 0
  jl    .error

  ; DELETE -> http_model_update
  mov   rdi, [rsp]
  mov   rsi, DELETE
  mov   rdx, http_model_route_id
  mov   rcx, http_model_delete
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
  add   rsp, BUFSIZ
  add   rsp, 0x20
  ret

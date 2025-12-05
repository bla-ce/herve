
section .text
; returns all instances of a model
; @param  rdi: pointer to the context struct
; @param  rsi: pointer to the model struct
; @return rax: return code
model_read_many:
  sub   rsp, 0x38

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> pointer to the model struct
  ; [rsp+0x10]  -> pointer to the linked list of instances
  ; [rsp+0x18]  -> pointer to the json object
  ; [rsp+0x20]  -> pointer to nested json object
  ; [rsp+0x28]  -> pointer to current instance
  ; [rsp+0x30]  -> pointer to json response object

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x18], 0
  mov   qword [rsp+0x20], 0
  mov   qword [rsp+0x30], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; get instances linked list
  mov   rdi, [rsp+0x8]
  call  model_get_instances
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax
  mov   [rsp+0x28], rax

  ; create json array
  call  json_array_create
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

.loop:
  cmp   qword [rsp+0x28], 0
  je    .loop_end

  ; convert instance to json object
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x28]
  call  model_instance_to_json
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  ; add json object
  mov   rdi, [rsp+0x18]
  mov   rsi, [rsp+0x20]
  call  json_array_insert_object
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; free json object
  mov   rdi, [rsp+0x20]
  call  json_free
  cmp   rax, 0
  jl    .error

  ; get next instance
  mov   rdi, [rsp+0x28]
  call  model_instance_get_next
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x28], rax

  jmp   .loop

.loop_end:
  mov   rdi, [rsp+0x18]
  call  json_array_end
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; create json response
  mov   rdi, TRUE
  xor   rsi, rsi
  mov   rdx, [rsp+0x18]
  call  create_json_response
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x30], rax

  ; send json
  mov   rdi, [rsp]
  mov   rsi, OK
  mov   rdx, [rsp+0x30]
  call  send_JSON
  cmp   rax, 0
  jl    .error

  ; free json response
  mov   rdi, [rsp+0x30]
  call  json_free
  cmp   rax, 0
  jl    .error

  ; free json object
  mov   rdi, [rsp+0x18]
  call  json_array_free
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rdi, [rsp+0x20]
  test  rdi, rdi
  jz    .free_obj

.free_obj:
  mov   rdi, [rsp+0x18]
  test  rdi, rdi
  jz    .no_free

  call  json_free

.no_free:
  ; create json response
  mov   rdi, FALSE
  mov   rsi, model_internal_server_err
  xor   rdx, rdx
  call  create_json_response
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x30], rax

  mov   rdi, [rsp]
  mov   rsi, INTERNAL_SERVER_ERROR
  mov   rdx, [rsp+0x30]
  call  send_JSON
  cmp   rax, 0
  jl    .error

  ; free json
  mov   rdi, [rsp+0x30]
  call  json_free
  cmp   rax, 0
  jl    .error

  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x38
  ret

; returns an instance of the model from its id
; @param  rdi: pointer to the context struct
; @param  rsi: pointer to the model struct
; @return rax: return code
model_read_single:
  sub   rsp, 0x38

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> pointer to the model struct
  ; [rsp+0x10]  -> instance id
  ; [rsp+0x18]  -> pointer to the linked list of instances
  ; [rsp+0x20]  -> json object of the instance
  ; [rsp+0x28]  -> pointer to the response message
  ; [rsp+0x30]  -> json object of the response

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x20], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; get context request
  mov   rdi, [rsp]
  call  get_ctx_request
  cmp   rax, 0
  jle   .error

  ; get id from param
  mov   rdi, rax
  mov   rsi, id_field_name
  call  get_dynamic_param
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  stoi
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; get instances linked list
  mov   rdi, [rsp+0x8]
  call  model_get_instances
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

.loop:
  cmp   qword [rsp+0x18], 0
  je    .loop_end

  ; get id of this instance
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x18]
  mov   rdx, id_field_name
  call  model_instance_get
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x10]
  cmp   rax, rdi
  je    .return_object

  ; get next instance
  mov   rdi, [rsp+0x18]
  call  model_instance_get_next
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  jmp   .loop

.loop_end:
  ; get model name
  mov   rdi, [rsp+0x8]
  call  model_get_name
  cmp   rax, 0
  jl    .error

  ; create message
  mov   rdi, rax
  mov   rsi, model_not_found_err
  call  strcat
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x28], rax

  ; create json response
  mov   rdi, FALSE
  mov   rsi, [rsp+0x28]
  xor   rdx, rdx
  call  create_json_response
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x30], rax

  mov   rdi, [rsp]
  mov   rsi, NOT_FOUND
  mov   rdx, [rsp+0x30]
  call  send_JSON
  cmp   rax, 0
  jl    .error

  ; free message
  mov   rdi, [rsp+0x28]
  call  free
  cmp   rax, 0
  jl    .error

  ; free json
  mov   rdi, [rsp+0x30]
  call  json_free
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.return_object:
  ; add instance to json
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x18]
  call  model_instance_to_json
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  ; create json response
  mov   rdi, TRUE
  xor   rsi, rsi
  mov   rdx, [rsp+0x20]
  call  create_json_response
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x30], rax

  ; send json
  mov   rdi, [rsp]
  mov   rsi, OK
  mov   rdx, [rsp+0x30]
  call  send_JSON
  cmp   rax, 0
  jl    .error

  ; free json response
  mov   rdi, [rsp+0x30]
  call  json_free
  cmp   rax, 0
  jl    .error

  ; free json object
  mov   rdi, [rsp+0x20]
  call  json_array_free
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rdi, [rsp+0x28]
  test  rdi, rdi
  jz    .free_json

  call  free

.free_json:
  mov   rdi, [rsp+0x20]
  test  rdi, rdi
  jz    .no_free

  call  json_free

.no_free:
  ; create json response
  mov   rdi, FALSE
  mov   rsi, model_internal_server_err
  xor   rdx, rdx
  call  create_json_response
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x30], rax

  mov   rdi, [rsp]
  mov   rsi, INTERNAL_SERVER_ERROR
  mov   rdx, [rsp+0x30]
  call  send_JSON
  cmp   rax, 0
  jl    .error

  ; free json
  mov   rdi, [rsp+0x30]
  call  json_free
  cmp   rax, 0
  jl    .error

  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x38
  ret

model_update:
  ret

; TODO: accept json, requires the parser to be implemented
; TODO: check for variable types
; creates an instance of the model from the form data
; @param  rdi: pointer to the context struct
; @param  rsi: pointer to the model struct
; @return rax: return code
model_post:
  sub   rsp, 0x48

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> pointer to the model struct
  ; [rsp+0x10]  -> pointer to the form data (hash table)
  ; [rsp+0x18]  -> number of fields
  ; [rsp+0x20]  -> array of ht keys
  ; [rsp+0x28]  -> pointer to the new instance
  ; [rsp+0x30]  -> dynamic pointer
  ; [rsp+0x38]  -> json of the instance
  ; [rsp+0x40]  -> json response

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x28], 0
  mov   qword [rsp+0x38], 0
  mov   qword [rsp+0x40], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

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

  mov   rdi, [rsp+0x10]
  call  ht_get_length
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; get list of fields
  mov   rdi, [rsp+0x10]
  call  ht_get_keys
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  ; create new instance of the model
  mov   rdi, [rsp+0x8]
  call  model_instance_create
  cmp   rax, 0
  jl    .error

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
  cmp   rax, TRUE
  jne   .unprocessable_entity

  ; get the value
  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp+0x30]
  call  ht_get

  ; set field value
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x28]
  mov   rdx, [rsp+0x30]
  mov   rcx, rax
  call  model_instance_set
  cmp   rax, 0
  jl    .internal_server_error

  jmp   .loop

.loop_end:
  ; add instance to json
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x28]
  call  model_instance_to_json
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x38], rax

  ; create json response
  mov   rdi, TRUE
  xor   rsi, rsi
  mov   rdx, [rsp+0x38]
  call  create_json_response
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x40], rax

  ; send json
  mov   rdi, [rsp]
  mov   rsi, CREATED
  mov   rdx, [rsp+0x40]
  call  send_JSON
  cmp   rax, 0
  jl    .error

  ; free json response
  mov   rdi, [rsp+0x40]
  call  json_free
  cmp   rax, 0
  jl    .error

  ; free json instance
  mov   rdi, [rsp+0x38]
  call  json_free
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.internal_server_error:
  ; TODO: return 500
  jmp   .error

.unprocessable_entity:
  ; TODO: return 422
  jmp   .error

.error:
  mov   rdi, [rsp+0x28]
  test  rdi, rdi
  jz    .free_json_instance

  call  free

.free_json_instance:
  mov   rdi, [rsp+0x38]
  test  rdi, rdi
  jz    .free_json

  call  json_free

.free_json:
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
model_delete:
  sub   rsp, 0x20

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> pointer to the model struct
  ; [rsp+0x10]  -> instance id
  ; [rsp+0x18]  -> pointer to the linked list of instances

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; get context request
  mov   rdi, [rsp]
  call  get_ctx_request
  cmp   rax, 0
  jle   .error

  ; get id from param
  mov   rdi, rax
  mov   rsi, id_field_name
  call  get_dynamic_param
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  stoi
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; get instances linked list
  mov   rdi, [rsp+0x8]
  call  model_get_instances
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

.loop:
  cmp   qword [rsp+0x18], 0
  je    .loop_end

  ; get id of this instance
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x18]
  mov   rdx, id_field_name
  call  model_instance_get
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x10]
  cmp   rax, rdi
  je    .remove_instance

  ; get next instance
  mov   rdi, [rsp+0x18]
  call  model_instance_get_next
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  jmp   .loop

.loop_end:
  mov   rdi, [rsp]
  mov   rsi, NOT_FOUND
  call  send_no_content
  cmp   rax, 0
  jl    .error

  ; return not found
  mov   rax, SUCCESS_CODE

  jmp   .return

.remove_instance:
  ; add instance to json
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x18]
  call  model_remove_instance
  cmp   rax, 0
  jl    .error

  ; send json
  mov   rdi, [rsp]
  mov   rsi, OK
  call  send_no_content
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  ; return 500
  mov   rdi, [rsp]
  mov   rsi, INTERNAL_SERVER_ERROR
  call  send_no_content
  cmp   rax, 0
  jl    .error

  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret

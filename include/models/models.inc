section .bss

model_t:
  _model_name       resq 1  ; pointer to the name of the field
  _model_fields     resq 1  ; linked list of field struct
  _model_curr_idx   resq 1  ; next index for the next instance
  _model_n_fields   resq 1  ; number of fields
  _model_instances  resq 1  ; linked list of instances
                            ; models are saved in memory for now
model_t_end:

section .data

MODEL_NAME_MAX_LEN  equ 0xFF

MODEL_T_LEN equ model_t_end - model_t

; offsets
MODEL_OFF_NAME      equ 0
MODEL_OFF_FIELDS    equ MODEL_OFF_NAME + 0x8
MODEL_OFF_CURR_IDX  equ MODEL_OFF_FIELDS + 0x8
MODEL_OFF_N_FIELDS  equ MODEL_OFF_CURR_IDX + 0x8
MODEL_OFF_INSTANCES equ MODEL_OFF_N_FIELDS + 0x8

model_not_found db " not found", NULL_CHAR

message_key db "message", NULL_CHAR
data_key    db "data", NULL_CHAR
success_key db "success", NULL_CHAR

section .text
; removes an instance from the linked list of the model
; @param  rdi: pointer to the model struct
; @param  rsi: pointer to the instance struct
; @return rax: return code
model_remove_instance:
  sub   rsp, 0x20

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the model struct
  ; [rsp+0x8]   -> pointer to the instance struct
  ; [rsp+0x10]  -> pointer to the prev instance
  ; [rsp+0x18]  -> pointer to the next instance

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   rdi, [rsp+0x8]
  call  model_instance_get_next
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  mov   rdi, [rsp+0x8]
  call  model_instance_get_prev
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  cmp   qword [rsp+0x18], 0
  je    .skip_set_next

  ; update next prev instance
  mov   rdi, [rsp+0x18]
  mov   rsi, [rsp+0x10]
  call  model_instance_set_next
  cmp   rax, 0
  jl    .error

.skip_set_next:
  cmp   qword [rsp+0x10], 0
  je    .skip_set_prev

  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp+0x18]
  call  model_instance_set_prev
  cmp   rax, 0
  jl    .error

.skip_set_prev:
  mov   rdi, [rsp+0x18]
  test  rdi, rdi
  jnz   .skip_update_head

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x10]
  call  model_set_instances
  cmp   rax, 0
  jl    .error

.skip_update_head:
  ; free instance
  mov   rdi, [rsp+0x8]
  call  model_instance_free
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret


; add an instance to the linked list of the model
; @param  rdi: pointer to the model struct
; @param  rsi: pointer to the instance struct
; @return rax: return code
model_add_instance:
  sub   rsp, 0x20

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the model struct
  ; [rsp+0x8]   -> pointer to the instance struct
  ; [rsp+0x10]  -> pointer to the linked list
  ; [rsp+0x18]  -> pointer to the prev instance

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x18], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   rdi, [rsp]
  add   rdi, MODEL_OFF_INSTANCES
  mov   [rsp+0x10], rdi

.loop:
  mov   rdi, [rsp+0x10]
  cmp   qword [rdi], 0
  je    .insert

  ; go to next
  mov   rsi, [rdi]
  mov   [rsp+0x18], rsi   ; save prev

  add   rsi, INSTANCE_OFF_NEXT

  mov   [rsp+0x10], rsi

  jmp   .loop

.insert:
  mov   rsi, [rsp+0x8]  ; pointer to the instance struct
  mov   [rdi], rsi      ; next pointer of prev instance

  mov   rdi, rsi
  mov   rsi, [rsp+0x18]
  call  model_instance_set_prev
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret

; sets the current index
; @param  rdi: pointer to the model
; @param  rsi: current index
; @return rax: return code
model_set_curr_idx:
  cmp   rdi, 0
  jl    .error

  cmp   rsi, 0
  jl    .error

  mov   [rdi+MODEL_OFF_CURR_IDX], rsi

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the current index
; @param  rdi: pointer to the model
; @return rax: current index
model_get_curr_idx:
  cmp   rdi, 0
  jl    .error

  mov   rax, [rdi+MODEL_OFF_CURR_IDX]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the number of fields the model has
; @param  rdi: pointer to the model
; @return rax: number of fields
model_get_n_fields:
  cmp   rdi, 0
  jl    .error

  mov   rax, [rdi+MODEL_OFF_N_FIELDS]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; generic function to insert any field to the model
; @param  rdi: pointer to the model struct
; @param  rsi: field name
; @param  rdx: field type
; @param  rcx: field spec arg
; @return rax: return code
model_insert_field:
  sub   rsp, 0x20

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the model struct
  ; [rsp+0x8]   -> pointer to the field name
  ; [rsp+0x10]  -> field type
  ; [rsp+0x18]  -> field spec arg

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   [rsp+0x18], rcx

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; check field type
  mov   rdi, [rsp+0x10]

  cmp   rdi, FIELD_TYPE_INTEGER
  je    .insert_number

  cmp   rdi, FIELD_TYPE_STRING
  je    .insert_string

  cmp   rdi, FIELD_TYPE_BOOL
  je    .insert_bool

  ; invalid type
  jmp   .error

.insert_number:
  jmp   .create_field

.insert_string:
  ; check max length of the string
  mov   rdi, [rsp+0x18]
  cmp   rdi, 0
  jle   .error

  jmp   .create_field

.insert_bool:
  jmp   .create_field

.create_field:
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x10]
  mov   rdx, [rsp+0x18]
  call  field_create
  cmp   rax, 0
  jl    .error

  ; add field to the linked list
  mov   rdi, [rsp]
  mov   rsi, rax
  call  model_add_field_to_list
  cmp   rax, 0
  jl    .error

  ; increase number of fields
  mov   rdi, [rsp]
  inc   qword [rdi+MODEL_OFF_N_FIELDS]

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret

; appends a field struct to the linked list of field
; @param  rdi: pointer to the model struct
; @param  rsi: pointer to the field struct
; @return rax: return code
model_add_field_to_list:
  sub   rsp, 0x18

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the model struct
  ; [rsp+0x8]   -> pointer to the field struct
  ; [rsp+0x10]  -> pointer to the linked list

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   rdi, [rsp]
  add   rdi, MODEL_OFF_FIELDS
  mov   [rsp+0x10], rdi

.loop:
  mov   rdi, [rsp+0x10]
  cmp   qword [rdi], 0
  je    .insert

  ; go to next
  mov   rsi, [rdi]
  add   rsi, FIELD_OFF_NEXT

  mov   [rsp+0x10], rsi

  jmp   .loop

.insert:
  mov   rsi, [rsp+0x8]
  mov   [rdi], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; creates a model
; @param  rdi: pointer to the name of the model
; @return rax: pointer to the model struct
model_create:
  sub   rsp, 0x10

  ; *** STACK USAGE *** ;
  ; [rsp]     -> pointer to the name of the model
  ; [rsp+0x8] -> pointer to the model struct

  mov   [rsp], rdi
  mov   qword [rsp+0x8], 0

  cmp   rdi, 0
  jl    .error

  mov   rdi, MODEL_T_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp]
  call  model_set_name
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x8]

  mov   qword [rax+MODEL_OFF_FIELDS], 0
  mov   qword [rax+MODEL_OFF_CURR_IDX], 0
  mov   qword [rax+MODEL_OFF_N_FIELDS], 0
  mov   qword [rax+MODEL_OFF_INSTANCES], 0

  ; insert id field
  mov   rdi, [rsp+0x8]
  mov   rsi, id_field_name
  mov   rdx, FIELD_TYPE_INTEGER
  call  model_insert_field
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x8]

  jmp   .return

.error:
  mov   rdi, [rsp+0x8]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; returns the linked list of fields
; @param  rdi: pointer to the model struct
; @return rax: pointer to the linked list
model_get_fields:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+MODEL_OFF_FIELDS]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the name of the model
; @param  rdi: pointer to the model struct
; @param  rsi: pointer to the name of the model
; @return rax: return code
model_set_name:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   [rdi+MODEL_OFF_NAME], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the name of the model
; @param  rdi: pointer to the model struct
; @return rax: pointer to the name of the model
model_get_name:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+MODEL_OFF_NAME]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; destroys the model and associates resources
; @param  rdi: pointer to the model struct
; @return rax: return code
model_free:
  sub   rsp, 0x10

  ; *** STACK USAGE *** ;
  ; [rsp]     -> pointer to the model struct
  ; [rsp+0x8] -> pointer to the fields linked list

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

  ; free fields
  add   rdi, MODEL_OFF_FIELDS
  mov   [rsp+0x8], rdi

.loop:
  mov   rax, [rsp+0x8]
  cmp   qword [rax], 0
  je    .loop_end

  ; get field struct
  mov   rdi, [rax]
  mov   [rsp+0x8], rdi
  call  free
  cmp   rax, 0
  jl    .error

  ; go to next
  mov   rdi, [rsp+0x8]
  add   rdi, FIELD_OFF_NEXT
  mov   [rsp+0x8], rdi

  jmp   .loop

.loop_end:
  ; free struct
  mov   rdi, [rsp]
  call  free
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; check if the field exist in the model
; @param  rdi: pointer to the model
; @param  rsi: pointer to the name of the field
; @return rax: model exist (bool)
model_field_exist:
  sub   rsp, 0x18

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the model
  ; [rsp+0x8]   -> pointer to the name of the field
  ; [rsp+0x10]  -> pointer to the fields

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  ; get fields
  add   rdi, MODEL_OFF_FIELDS

  mov   [rsp+0x10], rdi

.loop:
  mov   rax, [rsp+0x10]
  cmp   qword [rax], 0
  je    .end_loop

  ; compare fields
  mov   rsi, [rax]
  mov   rdi, [rsi+FIELD_OFF_NAME]
  mov   rsi, [rsp+0x8]
  call  strcmp
  cmp   rax, 0
  jl    .error

  cmp   rax, TRUE
  je    .exist

  ; go to next
  mov   rax, [rsp+0x10]
  mov   rdi, [rax]
  add   rdi, FIELD_OFF_NEXT
  mov   [rsp+0x10], rdi

  jmp   .loop

.end_loop:
  mov   rax, FALSE
  jmp   .return

.exist:
  mov   rax, TRUE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; sets the linked list of instances
; @param  rdi: pointer to the model struct
; @param  rsi: linked list of instances
; @return rax: return code
model_set_instances:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   [rdi+MODEL_OFF_INSTANCES], rsi

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the linked list of instances
; @param  rdi: pointer to the model struct
; @return rax: linked list of instances
model_get_instances:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+MODEL_OFF_INSTANCES]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns all instances of a model
; @param  rdi: pointer to the context struct
; @param  rsi: pointer to the model struct
; @return rax: return code
model_read_many:
  sub   rsp, 0x30

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> pointer to the model struct
  ; [rsp+0x10]  -> pointer to the linked list of instances
  ; [rsp+0x18]  -> pointer to the json object
  ; [rsp+0x20]  -> pointer to nested json object
  ; [rsp+0x28]  -> pointer to current instance

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x18], 0
  mov   qword [rsp+0x20], 0

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

  ; send json
  mov   rdi, [rsp]
  mov   rsi, OK
  mov   rdx, [rsp+0x18]
  call  send_JSON
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
  ; return 500
  mov   rdi, [rsp]
  mov   rsi, INTERNAL_SERVER_ERROR
  call  send_no_content
  cmp   rax, 0
  jl    .error

  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x30
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
  mov   rsi, model_not_found
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

  ; send json
  mov   rdi, [rsp]
  mov   rsi, OK
  mov   rdx, [rsp+0x20]
  call  send_JSON
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
  ; return 500
  mov   rdi, [rsp]
  mov   rsi, INTERNAL_SERVER_ERROR
  call  send_no_content
  cmp   rax, 0
  jl    .error

  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x38
  ret

model_update:
  ret

model_post:
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

; creates a json response with success, message and object
; user is responsible of freeing the json object
; @param  rdi: success (bool)
; @param  rsi: pointer to the message or 0
; @param  rdx: pointer to the object or 0
; @return rax: pointer to the json response object
create_json_response:
  sub   rsp, 0x20

  ; *** STACK USAGE *** ;
  ; [rsp]       -> success
  ; [rsp+0x8]   -> pointer to the message
  ; [rsp+0x10]  -> pointer to the object
  ; [rsp+0x18]  -> pointer to the response object

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   qword [rsp+0x18], 0

  cmp   rdi, FALSE
  jl    .error
  cmp   rdi, TRUE
  jg    .error

  cmp   rsi, 0
  jl    .error

  cmp   rdx, 0
  jl    .error

  call  json_create
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  mov   rdi, [rsp+0x18]
  mov   rsi, success_key
  mov   rdx, [rsp]
  call  json_insert_bool
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  mov   rdx, [rsp+0x8]
  test  rdx, rdx
  jz    .skip_message

  mov   rdi, [rsp+0x18]
  mov   rsi, message_key
  call  json_insert_string
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

.skip_message:
  mov   rdx, [rsp+0x10]
  test  rdx, rdx
  jz    .skip_data

  mov   rdi, [rsp+0x18]
  mov   rsi, data_key
  call  json_insert_object
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

.skip_data:
  mov   rdi, [rsp+0x18]
  call  json_end
  cmp   rax, 0
  jl    .error

  jmp   .return

.error:
  mov   rdi, [rsp+0x18]
  test  rdi, rdi
  jz    .no_free

  call  json_free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret

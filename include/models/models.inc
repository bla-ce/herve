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
; @param  r8: field is required
; @return rax: return code
model_insert_field:
  sub   rsp, 0x28

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the model struct
  ; [rsp+0x8]   -> pointer to the field name
  ; [rsp+0x10]  -> field type
  ; [rsp+0x18]  -> field spec arg
  ; [rsp+0x20]  -> field spec required

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   [rsp+0x18], rcx
  mov   [rsp+0x20], rcx

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
  ; TODO: validate boolean
  jmp   .create_field

.create_field:
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x10]
  mov   rdx, [rsp+0x18]
  mov   rcx, [rsp+0x20]
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
  add   rsp, 0x28
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
  xor   rcx, rcx
  mov   r8, TRUE
  call  model_insert_field
  cmp   rax, 0
  jl    .error

  ; insert created_at field
  mov   rdi, [rsp+0x8]
  mov   rsi, created_at_field_name
  mov   rdx, FIELD_TYPE_INTEGER
  xor   rcx, rcx
  ; this field is not required when creating/updating instance
  mov   r8, FALSE
  call  model_insert_field
  cmp   rax, 0
  jl    .error

  ; insert updated_at field
  mov   rdi, [rsp+0x8]
  mov   rsi, updated_at_field_name
  mov   rdx, FIELD_TYPE_INTEGER
  xor   rcx, rcx
  ; this field is not required when creating/updating instance
  mov   r8, FALSE
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
  mov   rdi, [rsp+0x10]

  mov   rax, [rdi]
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

; returns an instance of a model by ID
; @param  rdi: pointer to the model struct
; @param  rsi: id of the instance
; @return rax:
;   - pointer to the instance
;   - or 0 if does not exist
;   - or -1 if error
model_get_instance_by_id:
  sub   rsp, 0x18

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the model struct
  ; [rsp+0x8]   -> id of the instance
  ; [rsp+0x10]  -> linked list of the instances

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x10], 0

  ; get linked list of instances
  mov   rdi, [rsp]
  call  model_get_instances
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; loop over the instances and check the id
.loop:
  cmp   qword [rsp+0x10], 0
  je    .loop_end

  ; get id of this instance
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x10]
  mov   rdx, id_field_name
  call  model_instance_get
  cmp   rax, 0
  jl    .error

  cmp   rax, [rsp+0x8]
  je    .loop_end

  ; get next instance
  mov   rdi, [rsp+0x10]
  call  model_instance_get_next
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  jmp   .loop

.loop_end:
  mov   rax, [rsp+0x10]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

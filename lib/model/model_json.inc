section .text
; returns the json object of all instances of the model
; caller is responsible of freeing the json object
; @param  rdi: pointer to the model struct
; @return rax: pointer to the json object
model_instances_to_json:
  sub   rsp, 0x28

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the model struct
  ; [rsp+0x8]   -> pointer to the linked list of instances
  ; [rsp+0x10]  -> pointer to the json object
  ; [rsp+0x18]  -> pointer to nested json object
  ; [rsp+0x20]  -> pointer to current instance

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x10], 0
  mov   qword [rsp+0x18], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; get instances linked list
  mov   rdi, [rsp]
  call  model_get_instances
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax
  mov   [rsp+0x20], rax

  ; create json array
  call  json_array_create
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

.loop:
  cmp   qword [rsp+0x20], 0
  je    .loop_end

  ; convert instance to json object
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x20]
  call  model_instance_to_json
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; add json object
  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp+0x18]
  call  json_array_insert_object
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; free json object
  mov   rdi, [rsp+0x18]
  call  json_free
  cmp   rax, 0
  jl    .error

  ; get next instance
  mov   rdi, [rsp+0x20]
  call  model_instance_get_next
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  jmp   .loop

.loop_end:
  mov   rdi, [rsp+0x10]
  call  json_array_end
  cmp   rax, 0
  jl    .error

  jmp   .return

.error:
  mov   rdi, [rsp+0x18]
  test  rdi, rdi
  jz    .free_array

  call  json_free

.free_array:
  mov   rdi, [rsp+0x10]
  test  rdi, rdi
  jz    .no_free

  call  json_array_free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x28
  ret

; converts the model instance into a json object
; dev is responsible of freeing the json object
; @param  rdi: pointer to the model
; @param  rsi: pointer to the instance
; @return rax: pointer to the json object
model_instance_to_json:
  sub   rsp, 0x40

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the pointer to the model
  ; [rsp+0x8]   -> pointer to the pointer to the instance
  ; [rsp+0x10]  -> pointer to the json object
  ; [rsp+0x18]  -> pointer to the fields of the model
  ; [rsp+0x20]  -> pointer to the field struct
  ; [rsp+0x28]  -> name of the field
  ; [rsp+0x30]  -> type of the field
  ; [rsp+0x38]  -> field value of the instance

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x10], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  call  json_create
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; get fields of the model
  mov   rdi, [rsp]
  add   rdi, MODEL_OFF_FIELDS

  mov   [rsp+0x18], rdi

.loop:
  mov   rdi, [rsp+0x18]
  cmp   qword [rdi], 0
  je    .loop_end

  mov   rax, [rdi]
  mov   [rsp+0x20], rax
  mov   [rsp+0x18], rax

  ; get name of the field
  mov   rdi, [rsp+0x20]
  call  field_get_name
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x28], rax

  ; get type of the field
  mov   rdi, [rsp+0x20]
  call  field_get_type
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x30], rax

  ; get value
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  mov   rdx, [rsp+0x28]
  call  model_instance_get_value
  cmp   qword [MODEL_INSTANCE_ERR], TRUE
  je    .go_next

  mov   [rsp+0x38], rax

  ; check the type of the field
  mov   rdi, [rsp+0x30]
  cmp   rdi, FIELD_TYPE_INTEGER
  je    .insert_number

  cmp   rdi, FIELD_TYPE_STRING
  je    .insert_string

  cmp   rdi, FIELD_TYPE_BOOL
  je    .insert_bool

  cmp   rdi, FIELD_TYPE_FLOAT
  je    .insert_float

  ; unknown type
  ; TODO: logger
  jmp   .error

.insert_number:
  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp+0x28]
  mov   rdx, [rsp+0x38]
  call  json_insert_integer
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  jmp   .go_next

.insert_string:
  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp+0x28]
  mov   rdx, [rsp+0x38]
  call  json_insert_string
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  jmp   .go_next

.insert_bool:
  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp+0x28]
  mov   rdx, [rsp+0x38]
  call  json_insert_bool
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  jmp   .go_next

.insert_float:
  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp+0x28]
  mov   rdx, [rsp+0x38]
  call  json_insert_float
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  jmp   .go_next

.go_next:
  ; go to next field
  mov   rdi, [rsp+0x18]
  add   rdi, FIELD_OFF_NEXT

  mov   [rsp+0x18], rdi
  jmp   .loop

.loop_end:
  mov   rdi, [rsp+0x10]
  call  json_end
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  jmp   .return

.error:
  mov   rdi, [rsp+0x10]
  test  rdi, rdi
  jz    .no_free

  call  json_free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x40
  ret

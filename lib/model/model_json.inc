section .text
; returns the json object of all instances of the model
; caller is responsible of freeing the json object
; @param  rdi: pointer to the model struct
; @return rax: pointer to the json object
model_instances_to_json:
  sub   rsp, 0x28

  ; STACK USAGE
  ; [rsp]       -> pointer to the model struct
  ; [rsp+0x8]   -> pointer to the linked list of instances
  ; [rsp+0x10]  -> pointer to the json object
  ; [rsp+0x18]  -> pointer to nested json object
  ; [rsp+0x20]  -> pointer to stringified nested json object

  mov   [rsp], rdi
  mov   qword [rsp+0x10], 0
  mov   qword [rsp+0x18], 0
  mov   qword [rsp+0x20], 0

  test  rdi, rdi
  jz    .error

  ; get instances linked list
  mov   rdi, [rsp]
  call  model_get_instances
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; create json array
  call  json_array_create
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

.loop:
  cmp   qword [rsp+0x8], 0
  je    .loop_end

  mov   rdi, [rsp+0x8]
  call  linked_list_get_node_data
  cmp   byte [linked_list_errno], TRUE
  je    .error

  ; convert instance to json object
  mov   rdi, [rsp]
  mov   rsi, rax
  call  model_instance_to_json
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  mov   rdi, rax
  call  json_to_str
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  ; add json object
  mov   rdi, [rsp+0x10]
  mov   rsi, rax
  call  json_array_insert_object
  cmp   rax, 0
  jl    .error

  ; free json object
  mov   rdi, [rsp+0x18]
  call  json_free
  cmp   rax, 0
  jl    .error

  ; free json to str
  mov   rdi, [rsp+0x20]
  call  free
  cmp   rax, 0
  jl    .error

  ; get next instance
  mov   rdi, [rsp+0x8]
  call  linked_list_get_node_next
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   [rsp+0x8], rax

  test  rax, rax
  jnz   .loop

.loop_end:
  mov   rdi, [rsp+0x10]
  call  json_array_end
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x10]

  jmp   .return

.error:
  mov   rdi, [rsp+0x20]
  test  rdi, rdi
  jz    .free_json_object

  call  free

.free_json_object:
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

  ; STACK USAGE
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

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  call  json_create
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; get fields of the model
  mov   rdi, [rsp]
  mov   rax, [rdi+MODEL_OFF_FIELDS]

  mov   [rsp+0x18], rax

.loop:
  cmp   qword [rsp+0x18], 0
  je    .loop_end

  mov   rdi, [rsp+0x18]
  call  linked_list_get_node_data
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   [rsp+0x20], rax

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
  cmp   byte [model_instance_errno], TRUE
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

  jmp   .go_next

.insert_string:
  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp+0x28]
  mov   rdx, [rsp+0x38]
  call  json_insert_string
  cmp   rax, 0
  jl    .error

  jmp   .go_next

.insert_bool:
  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp+0x28]
  mov   rdx, [rsp+0x38]
  call  json_insert_bool
  cmp   rax, 0
  jl    .error

  jmp   .go_next

.insert_float:
  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp+0x28]
  mov   rdx, [rsp+0x38]
  call  json_insert_float
  cmp   rax, 0
  jl    .error

  jmp   .go_next

.go_next:
  mov   rdi, [rsp+0x18]
  call  linked_list_get_node_next
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   [rsp+0x18], rax

  jmp   .loop

.loop_end:
  mov   rdi, [rsp+0x10]
  call  json_end
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x10]

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

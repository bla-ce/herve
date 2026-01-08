section .bss

instance_t:
  _instance_ht    resq 1  ; pointer to the hash table
  _instance_next  resq 1  ; pointer to the next instance
  _instance_prev  resq 1  ; pointer to the prev instance
instance_t_end:

section .data

INSTANCE_T_LEN equ instance_t_end - instance_t

; offsets
INSTANCE_OFF_HT   equ 0
INSTANCE_OFF_NEXT equ INSTANCE_OFF_HT + 0x8
INSTANCE_OFF_PREV equ INSTANCE_OFF_NEXT + 0x8

section .text
; validates and saves an instance to the model
; if strict mode is not set, the validation will be skipped
; @param  rdi: pointer the model struct
; @param  rsi: pointer to the instance
; @param  rdx: strict mode
; @return rax: return code
model_instance_save:
; TODO: not implemented
  sub   rsp, 0x18

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the model struct
  ; [rsp+0x8]   -> pointer to the instance
  ; [rsp+0x10]  -> strict mode flag

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; TODO: validate boolean argument

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx

  cmp   qword [rsp+0x10], FALSE
  je    .skip_validation

  ; mov   rdi, [rsp]
  ; mov   rsi, [rsp+0x8]
  ; call  model_instance_validate
  ; cmp   rax, 0
  ; jl    .error

.skip_validation:
  ; add the instance to linked list
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  call  model_add_instance
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; validates an instance of a particular model
; @param  rdi: pointer the model struct
; @param  rsi: pointer to the instance
; @return rax:
  ; TRUE: instance is valid
  ; FALSE: instance is invalid
  ; -1 if an error occurs
model_instance_validate:
  ; TODO: implement
  mov   rax, FAILURE_CODE
  ret

; creates a new instance of the new model
; @param  rdi: pointer to the model struct
; @return rax: new instance of the model
model_instance_create:
  sub   rsp, 0x18

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the model struct
  ; [rsp+0x8]   -> pointer to the instance struct
  ; [rsp+0x10]  -> pointer to the hash table

  mov   [rsp], rdi
  mov   qword [rsp+0x8], 0
  mov   qword [rsp+0x10], 0

  cmp   rdi, 0
  jl    .error

  ; malloc instance struct
  mov   rdi, INSTANCE_T_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   qword [rax+INSTANCE_OFF_NEXT], 0
  mov   qword [rax+INSTANCE_OFF_PREV], 0

  ; get number of fields
  mov   rdi, [rsp]
  call  model_get_n_fields
  cmp   rax, 0
  jl    .error

  ; in memory, the instances are hash tables
  mov   rdi, rax
  call  ht_create
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  model_instance_set_ht
  cmp   rax, 0
  jl    .error

  ; get the current id
  mov   rdi, [rsp]
  call  model_get_curr_idx
  cmp   rax, 0
  jl    .error

  ; set the id
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  mov   rdx, id_field_name
  mov   rcx, rax
  call  model_instance_set
  cmp   rax, 0
  jl    .error

  ; increase current id
  mov   rdi, [rsp]
  call  model_get_curr_idx
  cmp   rax, 0
  jl    .error

  inc   rax

  mov   rdi, [rsp]
  mov   rsi, rax
  call  model_set_curr_idx
  cmp   rax, 0
  jl    .error

  ; get now
  call  now
  cmp   rax, 0
  jl    .error

  ; set created_at
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  mov   rdx, created_at_field_name
  mov   rcx, rax
  call  model_instance_set
  cmp   rax, 0
  jl    .error

  ; set updated_at to 0
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  mov   rdx, updated_at_field_name
  xor   rcx, rcx
  call  model_instance_set
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x8]

  jmp   .return

.error:
  mov   rdi, [rsp+0x8]
  test  rdi, rdi
  jz    .free_ht

  call  free

.free_ht:
  mov   rdi, [rsp+0x10]
  test  rdi, rdi
  jz    .no_free

  call  ht_free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; sets a value of a field for the instance
; @param  rdi: pointer to the model
; @param  rsi: pointer to the instance
; @param  rdx: pointer to the name of the field
; @param  rcx: value
; @return rax: return code
model_instance_set:
  sub   rsp, 0x20

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the model
  ; [rsp+0x8]   -> pointer to the instance
  ; [rsp+0x10]  -> pointer to the name of the field
  ; [rsp+0x18]  -> value

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   [rsp+0x18], rcx

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  cmp   rdx, 0
  jle   .error

  ; check if field exist
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x10]
  call  model_field_exist
  cmp   rax, 0
  jle   .error

  ; get hash table
  mov   rdi, [rsp+0x8]
  call  model_instance_get_ht
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, [rsp+0x10]
  mov   rdx, [rsp+0x18]
  call  ht_insert
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret

; returns the value of a field for the instance
; @param  rdi: pointer to the model
; @param  rsi: pointer to the instance
; @param  rdx: pointer to the name of the field
; @return rax: value
model_instance_get:
  sub   rsp, 0x18

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the model
  ; [rsp+0x8]   -> pointer to the instance
  ; [rsp+0x10]  -> pointer to the name of the field

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  cmp   rdx, 0
  jle   .error

  ; check if field exist
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x10]
  call  model_field_exist
  cmp   rax, 0
  jle   .error

  mov   rdi, [rsp+0x8]
  call  model_instance_get_ht
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, [rsp+0x10]
  call  ht_get
  cmp   qword [HT_ERR_MISSING_KEY], TRUE
  je    .error

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; frees an instance of the model
; @param  rdi: pointer to the instance
; @return rax: return code
model_instance_free:
  sub   rsp, 0x8

  ; *** STACK FREE *** ;
  ; [rsp]   -> pointer to the instance struct

  mov   [rsp], rdi

  cmp   rdi, 0
  jl    .error

  mov   rdi, [rsp]
  call  model_instance_get_ht
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  ht_free
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

; sets the hash table of the instance
; @param  rdi: pointer to the instance struct
; @param  rsi: pointer to the hash table
; @return rax: return code
model_instance_set_ht:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   [rdi+INSTANCE_OFF_HT], rsi

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the hash table of the instance
; @param  rdi: pointer to the instance
; @return rax: pointer to the hash table
model_instance_get_ht:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+INSTANCE_OFF_HT]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the prev instance of the linked list
; @param  rdi: pointer to the instance
; @param  rsi: pointer to the prev instance
; @return rax: return code
model_instance_set_prev:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   [rdi+INSTANCE_OFF_PREV], rsi

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the prev instance of the linked list
; @param  rdi: pointer to the instance
; @return rax: pointer to the prev instance
model_instance_get_prev:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+INSTANCE_OFF_PREV]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the next instance of the linked list
; @param  rdi: pointer to the instance
; @param  rsi: pointer to the next instance
; @return rax: return code
model_instance_set_next:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   [rdi+INSTANCE_OFF_NEXT], rsi

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the next instance of the linked list
; @param  rdi: pointer to the instance
; @return rax: pointer to the next instance
model_instance_get_next:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+INSTANCE_OFF_NEXT]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
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
  call  model_instance_get

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
  je    .insert_bool

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

section .bss

instance_t:
  .instance_ht    resq 1  ; pointer to the hash table
  .instance_next  resq 1  ; pointer to the next instance
  .instance_prev  resq 1  ; pointer to the prev instance
instance_t_end:

section .data

INSTANCE_T_LEN equ instance_t_end - instance_t

; offsets
INSTANCE_OFF_HT   equ 0
INSTANCE_OFF_NEXT equ INSTANCE_OFF_HT + 0x8
INSTANCE_OFF_PREV equ INSTANCE_OFF_NEXT + 0x8

MODEL_INSTANCE_ERR dq 0

section .text
; updates an instance of the model
; non existent field are ignored for now
; TODO: warn or error on the above?
; @param  rdi: pointer to the model struct
; @param  rsi: pointer to the instance to update
; @param  rdx: hash table containing the updates
; @param  rcx: values need to be converted from str (bool)
; @return rax: return code
model_instance_update:
  sub   rsp, 0x30

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the model struct
  ; [rsp+0x8]   -> pointer to the instance to update
  ; [rsp+0x10]  -> hash table containing the updates
  ; [rsp+0x18]  -> values need to be converted from string
  ; [rsp+0x20]  -> pointer to the fields linked list
  ; [rsp+0x28]  -> pointer to the name of the field

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

  ; TODO: validate bool arg

  mov   rdi, [rsp]
  call  model_get_fields
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

.loop:
  mov   rdi, [rsp+0x20]
  call  field_get_name
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x28], rax

  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp+0x28]
  call  ht_get
  cmp   qword [HT_ERR_MISSING_KEY], TRUE
  je   .skip_update

  cmp   qword [rsp+0x20], FALSE
  je    .skip_conversion

  ; convert the value to the right type
  mov   rdi, [rsp+0x20]
  mov   rsi, rax
  call  field_convert_str_to_value
  cmp   rax, 0
  jl    .error

.skip_conversion:
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  mov   rdx, [rsp+0x28]
  mov   rcx, rax
  call  model_instance_set_value
  cmp   rax, 0
  jl    .error

.skip_update:
  ; go to next field
  mov   rsi, [rsp+0x20]
  mov   rdi, qword [rsi+FIELD_OFF_NEXT]
  mov   [rsp+0x20], rdi

  test  rdi, rdi
  jz    .loop_end

  jmp   .loop

.loop_end:
  ; updated_at field
  call  now
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  mov   rdx, updated_at_field_name
  mov   rcx, rax
  call  model_instance_set_value
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x30
  ret

; validates and saves an instance to the model
; if strict mode is not set, the validation will be skipped
; @param  rdi: pointer to the model struct
; @param  rsi: pointer to the instance
; @param  rdx: strict mode
; @return rax: return code
model_instance_save:
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

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  call  model_instance_validate
  cmp   rax, 0
  jl    .error

  cmp   rax, FALSE  ; instance is invalid
  je    .error

.skip_validation:
  ; add the instance to linked list
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  call  model_add_instance
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
; @return rax:
  ; TRUE: instance is valid
  ; FALSE: instance is invalid
  ; -1 if an error occurs
  ; FALSE: instance is invalid
  ; -1 if an error occurs
model_instance_validate:
  sub   rsp, 0x20

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the model struct
  ; [rsp+0x8]   -> pointer to the instance
  ; [rsp+0x10]  -> linked list of model fields
  ; [rsp+0x18]  -> value of the field being validated

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; get list of fields
  call  model_get_fields
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

.loop:
  ; get the name of the field
  mov   rdi, [rsp+0x10]
  call  field_get_name
  cmp   rax, 0
  jl    .error

  ; check if the field is present
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  mov   rdx, rax
  call  model_instance_get_value
  ; will check global after

  mov   [rsp+0x18], rax

  ; check if the field is required
  mov   rdi, [rsp+0x10]
  call  field_get_required
  cmp   rax, 0
  jl    .error
  cmp   rax, FALSE
  je    .not_required

  cmp   qword [MODEL_INSTANCE_ERR], TRUE
  je    .is_invalid

.not_required:
  cmp   qword [MODEL_INSTANCE_ERR], TRUE
  je    .skip_validation

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  mov   rdx, [rsp+0x18]
  call  _model_instance_validate_field
  cmp   rax, 0
  jl    .error
  je    .is_invalid

.skip_validation:
  ; go to next
  mov   rdi, [rsp+0x10]
  call  field_get_next
  cmp   rax, 0
  jl    .error
  je    .loop_end

  mov   [rsp+0x10], rax

  jmp   .loop

.loop_end:

.is_valid:
  mov   rax, TRUE
  jmp   .return

.is_invalid:
  mov   rax, FALSE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret

; validates a specific field of an instance
; @param  rdi: pointer to the model of the instance
; @param  rsi: pointer to the field struct
; @param  rdx: value
; @return rax:
  ; TRUE: instance is valid
  ; FALSE: instance is invalid
  ; -1 if an error occurs
_model_instance_validate_field:
  sub   rsp, 0x20

  ; *** STACKE USAGE *** ;
  ; [rsp]       -> pointer to the model of the instance
  ; [rsp+0x8]   -> pointer to the field struct
  ; [rsp+0x10]  -> pointer to the value
  ; [rsp+0x18]  -> max length of the string

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; get type of the field
  mov   rdi, [rsp+0x8]
  call  field_get_type
  cmp   rax, 0
  jl    .error

  cmp   rax, FIELD_TYPE_STRING
  je    .is_string

  cmp   rax, FIELD_TYPE_BOOL
  je    .is_boolean

  jmp   .is_valid ; valid by default

.is_string:
  ; get max length of the string
  mov   rdi, [rsp+0x8]
  call  field_get_arg
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; get length of the string
  mov   rdi, [rsp+0x10]
  call  strlen
  cmp   rax, 0
  jl    .is_invalid

  cmp   rax, [rsp+0x18]
  je    .is_invalid

  jmp   .is_valid

.is_boolean:
  cmp   qword [rsp+0x10], TRUE
  je    .is_valid

  cmp   qword [rsp+0x10], FALSE
  je    .is_valid

  jmp   .is_invalid

.is_valid:
  mov   rax, TRUE
  jmp   .return

.is_invalid:
  mov   rax, FALSE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
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
  call  model_instance_set_value
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
  call  model_instance_set_value
  cmp   rax, 0
  jl    .error

  ; set updated_at to 0
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  mov   rdx, updated_at_field_name
  mov		rcx, NO_ARG
  call  model_instance_set_value
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
model_instance_set_value:
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

  ; convert value to string for ht
  mov   rdi, rax
  mov   rsi, [rsp+0x18]
  call  field_convert_value_to_str
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

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

  ; free the value
  mov   rdi, [rsp+0x18]
  call  free
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
model_instance_get_value:
  sub   rsp, 0x20

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the model
  ; [rsp+0x8]   -> pointer to the instance
  ; [rsp+0x10]  -> pointer to the name of the field
  ; [rsp+0x18]  -> pointer to the field struct

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

  mov   [rsp+0x18], rax

  mov   rdi, [rsp+0x8]
  call  model_instance_get_ht
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, [rsp+0x10]
  call  ht_get
  cmp   qword [HT_ERR_MISSING_KEY], TRUE
  je    .error

  ; this value is a string, convert back to the field type
  mov   rdi, [rsp+0x18]
  mov   rsi, rax
  call  field_convert_str_to_value
  cmp   rax, 0
  jl    .error

  mov   qword [MODEL_INSTANCE_ERR], FALSE
  jmp   .return

.error:
  mov   qword [MODEL_INSTANCE_ERR], TRUE
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
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

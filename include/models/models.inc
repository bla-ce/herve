section .bss

model_t:
  model_t_name    resq 1
  model_t_fields  resq 1  ; linked list of fields
model_t_end:

section .data

MODEL_T_LEN equ model_t_end - model_t

; model offsets
MODEL_OFF_NAME    equ 0
MODEL_OFF_FIELDS  equ MODEL_OFF_NAME + 0x8

section .text

; initializes a model
; @param  rdi: pointer to the name of the model
; @return rax: pointer to the model struct
model_init:
  sub   rsp, 0x18

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the name of the model 
  ; [rsp+0x8]   -> pointer to allocated model struct
  ; [rsp+0x10]  -> pointer to duplicated string

  mov   [rsp], rdi
  mov   qword [rsp+0x8], 0
  mov   qword [rsp+0x10], 0

  cmp   rdi, 0
  jle   .error

  ; allocate model struct
  mov   rdi, MODEL_T_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; duplicate string
  mov   rdi, [rsp]
  call  strdup
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  model_set_name
  cmp   rax, 0
  jl    .error

  ; create id field
  mov   rdi, [rsp+0x8]
  mov   rsi, field_id_name
  mov   rdx, FIELD_OFF_INT
  call  model_insert_int
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x8]

  jmp   .return

.error:
  mov   rdi, [rsp+0x10]
  test  rdi, rdi
  jz    .free_struct

  call  free 

.free_struct:
  mov   rdi, [rsp+0x8]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; sets the name of the model
; @param  rdi: pointer to the model
; @param  rsi: pointer to the name of the model
; @return rax: return code
model_set_name:
  cmp   rdi, 0
  jle   .error
  
  cmp   rsi, 0
  jl    .error

  mov   [rdi+MODEL_OFF_NAME], rsi

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret
  
; returns the name of the model
; @param  rdi: pointer to the model
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

; sets the linked list of fields of the model
; @param  rdi: pointer to the model
; @param  rsi: pointer to the linked list of fields
; @return rax: return code
model_set_fields:
  cmp   rdi, 0
  jle   .error
  
  cmp   rsi, 0
  jl    .error

  mov   [rdi+MODEL_OFF_FIELDS], rsi

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret
  
; returns the linked list of fields of the model
; @param  rdi: pointer to the model
; @return rax: pointer to the linked list of the model
model_get_fields:
  cmp   rdi, 0
  jle   .error
  
  mov   rax, [rdi+MODEL_OFF_FIELDS]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; frees the model struct and associated resources
; @param  rdi: pointer to the model struct
; @return rax: return code
model_free:
  sub   rsp, 0x8

  ; *** STACK USAGE *** ;
  ; [rsp] -> pointer to the model struct

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

  ; free the name of the model
  mov   rdi, [rsp]
  call  model_get_name
  cmp   rax, 0
  jl    .error
  
  call  free
  cmp   rax, 0
  jl    .error

  ; TODO: free fields

  ; free the structure
  mov   rdi, [rsp]
  call  free

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; inserts a field to a model
; @param  rdi: pointer to the model struct
; @param  rsi: pointer to the name of the field
; @param  rdx: field type
; @param  rcx: specific argument for type
; @return rax: return code
model_insert_field:
  sub   rsp, 0x28

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the model struct
  ; [rsp+0x8]   -> pointer to the name of the field
  ; [rsp+0x10]  -> field type
  ; [rsp+0x18]  -> argument for type
  ; [rsp+0x20]  -> pointer to the field struct

  mov   [rsp], rdi 
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   [rsp+0x18], rcx
  mov   qword [rsp+0x20], 0

  cmp   rdi, 0
  jle   .error
  
  cmp   rsi, 0
  jle   .error

  cmp   rdx, 0
  jl    .error

  ; check type
  cmp   rdx, FIELD_OFF_STRING
  je    .insert_string

  cmp   rdx, FIELD_OFF_INT
  je    .insert_string

  cmp   rdx, FIELD_OFF_BOOL
  je    .insert_string

  ; field not supported
  jmp   .error

.insert_string:
  ; all registers are already populated
  cmp   rcx, 0
  jle   .error

  mov   rdx, rcx

  call  model_insert_string
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  jmp   .insert_field

.insert_integer:
  ; all registers are already populated
  call  model_insert_int
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  jmp   .insert_field

.insert_bool:
  ; all registers are already populated
  call  model_insert_bool
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  jmp   .insert_field

.insert_field:
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x20]
  call  add_field_to_list
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x20]

  jmp   .return

.error:
  mov   rdi, [rsp+0x20]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x28
  ret

; adds the field to the linked list
; @param  rdi: pointer to the model struct
; @param  rsi: pointer to the field struct
; @return rax: return code
add_field_to_list:
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

  call  model_get_fields
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  test  rax, rax
  jnz   .loop

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  call  model_set_fields
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.loop:
  ; get next struct
  mov   rdi, [rsp+0x10]
  call  field_get_next
  cmp   rax, 0
  jl    .error

  test  rax, rax
  jz    .loop_end

  ; go to next
  mov   rdi, [rsp+0x10]
  mov   rax, [rdi+FIELD_OFF_NEXT]

  mov   [rsp+0x10], rax

  jmp   .loop

.loop_end:
  ; add the field to next
  mov   rdi, [rsp+0x10]

  mov   rsi, [rsp+0x8]
  call  field_set_next
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

section .bss
section .data
section .text
; creates a new instance of the new model
; @param  rdi: pointer to the model struct
; @return rax: new instance of the model
model_instance_create:
  sub   rsp, 0x10

  ; *** STACK USAGE *** ;
  ; [rsp]     -> pointer to the model struct
  ; [rsp+0x8] -> pointer to the instance

  mov   [rsp], rdi
  mov   qword [rsp+0x8], 0

  cmp   rdi, 0
  jl    .error

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

  mov   [rsp+0x8], rax

  ; set the id
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

  mov   rax, [rsp+0x8]

  jmp   .return

.error:
  mov   rdi, [rsp+0x8]
  test  rdi, rdi
  jz    .no_free

  call  ht_free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
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
  jl    .error

  cmp   rax, TRUE
  jne   .error

  mov   rdi, [rsp+0x8]
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

; frees an instance of the model
; @param  rdi: pointer to the instance
; @return rax: return code
model_instance_free:
  cmp   rdi, 0
  jl    .error

  call  ht_free
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE
   
.return:
  ret


section .bss

model_t:
  name  dq 0
model_t_end:

section .data

MODEL_T_LEN equ model_t_end - model_t

; offsets
MODEL_OFF_NAME equ 0

section .text

; initializes a model
; @param  rdi: pointer to the name of the model
; @return rax: pointer to the model struct
model_init:
  sub   rsp, 0x10

  ; *** STACK USAGE *** ;
  ; [rsp]     -> pointer to the name of the model 
  ; [rsp+0x8] -> pointer to allocated model struct

  mov   [rsp], rdi
  mov   qword [rsp+0x8], 0

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

  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  model_set_name
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x8]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
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

  ; free the structure
  mov   rdi, [rsp]
  call  free

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

section .bss

model_t:
  _model_name    resq 1  ; pointer to the name of the field
  _model_fields  resq 1  ; linked list of field struct
model_t_end:

section .data

MODEL_NAME_MAX_LEN  equ 0xFF

MODEL_T_LEN equ model_t_end - model_t

; offsets
MODEL_OFF_NAME    equ 0
MODEL_OFF_FIELDS  equ MODEL_OFF_NAME + 0x8

section .text
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
  sub   rsp, 0x8

  ; *** STACK USAGE *** ;
  ; [rsp]   -> pointer to the model struct

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

  ; free struct
  mov   rdi, [rsp]
  call  free
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret


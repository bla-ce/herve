section .data

model_name  db "user", NULL_CHAR

field_username db "username", NULL_CHAR
field_password db "password", NULL_CHAR
field_age      db "age", NULL_CHAR
field_active   db "active", NULL_CHAR

section .text

; @return rax: pointer to the user model
create_user_model:
  sub   rsp, 0x8

  ; *** STACK USAGE *** ;
  ; [rsp] -> pointer to the user model

  mov   rdi, model_name
  call  model_create
  cmp   rax, 0
  jl    .error

  mov   [rsp], rax

  mov   rdi, [rsp]
  mov   rsi, field_username
  mov   rdx, FIELD_TYPE_STRING
  mov   rcx, 32
  mov   r8, TRUE
  call  model_insert_field
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, field_password
  mov   rdx, FIELD_TYPE_STRING
  mov   rcx, 128
  mov   r8, TRUE
  call  model_insert_field
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, field_age
  mov   rdx, FIELD_TYPE_INTEGER
  mov   r8, TRUE
  call  model_insert_field
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, field_active
  mov   rdx, FIELD_TYPE_BOOL
  mov   r8, TRUE
  call  model_insert_field
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

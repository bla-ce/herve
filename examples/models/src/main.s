global _start

%include "herve.inc"
%include "models.s"

section .text
_start:
  call  server_init
  cmp   rax, 0
  jl    .error

  mov   [server], rax

  mov   rdi, [server]
  call  server_enable_logger
  cmp   rax, 0
  jl    .error

  call  create_user_model
  cmp   rax, 0
  jl    .error

  mov   [person_model], rax

  mov   rdi, [person_model]
  call  model_instance_create
  cmp   rax, 0
  jl    .error

  mov   [instance1], rax

  mov   rdi, [person_model]
  mov   rsi, [instance1]
  mov   rdx, field_username
  mov   rcx, user1
  call  model_instance_set
  cmp   rax, 0
  jl    .error

  mov   rdi, [person_model]
  mov   rsi, [instance1]
  mov   rdx, field_password
  mov   rcx, pass1
  call  model_instance_set
  cmp   rax, 0
  jl    .error

  mov   rdi, [person_model]
  call  model_instance_create
  cmp   rax, 0
  jl    .error

  mov   [instance2], rax

  mov   rdi, [person_model]
  mov   rsi, [instance2]
  mov   rdx, field_username
  mov   rcx, user2
  call  model_instance_set
  cmp   rax, 0
  jl    .error

  mov   rdi, [person_model]
  mov   rsi, [instance2]
  mov   rdx, field_password
  mov   rcx, pass2
  call  model_instance_set
  cmp   rax, 0
  jl    .error

  mov   rdi, [server]
  mov   rsi, [person_model]
  call  add_model_routes
  cmp   rax, 0
  jl    .error

  mov   rdi, [server]
  call  server_run

  mov   rdi, SUCCESS_CODE
  call  exit

.error:
  mov   rdi, FAILURE_CODE
  call  exit

section .data
  server dq 0

  user1 db "user1", NULL_CHAR
  pass1 db "pass1", NULL_CHAR

  user2 db "user2", NULL_CHAR
  pass2 db "pass2", NULL_CHAR

  instance1 dq 0
  instance2 dq 0

  person_model dq 0

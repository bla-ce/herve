global _start

%include "herve.inc"

section .text
_start:
  call  server_init
  cmp   rax, 0
  jl    .error

  mov   [proxy], rax

  ; create array of upstream servers
  mov   rdi, 2
  mov   rsi, 8
  call  array_create
  cmp   rax, 0
  jl    .error

  mov   [target1_array], rax

  mov   rdi, 1
  mov   rsi, 8
  call  array_create
  cmp   rax, 0
  jl    .error

  mov   [target2_array], rax

  ; create first proxy target struct 
  mov   rdi, target1_port
  mov   rsi, target1_id
  mov   rdx, target1_weight
  call  proxy_target_create
  cmp   rax, 0
  jl    .error

  mov   [target1], rax

  ; create second proxy target struct 
  mov   rdi, target2_port
  mov   rsi, target2_id
  call  proxy_target_create
  cmp   rax, 0
  jl    .error

  mov   [target2], rax

  mov   rdi, target3_port
  mov   rsi, target3_id
  mov   rdx, target3_weight
  call  proxy_target_create
  cmp   rax, 0
  jl    .error

  mov   [target3], rax

  mov   rdi, [target1_array]
  mov   rsi, target1
  call  array_push
  cmp   rax, 0
  jl    .error

  mov   rdi, [target1_array]
  mov   rsi, target3
  call  array_push
  cmp   rax, 0
  jl    .error

  mov   rdi, [target2_array]
  mov   rsi, target2
  call  array_push
  cmp   rax, 0
  jl    .error

  ; create group
  mov   rdi, [proxy]
  mov   rsi, target1_url
  mov   rdx, FALSE
  call  add_group
  cmp   rax, 0
  jl    .error

  mov   [target1_group], rax

  mov   rdi, [proxy]
  mov   rsi, target2_url
  mov   rdx, FALSE
  call  add_group
  cmp   rax, 0
  jl    .error

  mov   [target2_group], rax

  ; create middleware
  mov   rdi, proxy_middleware
  mov   rsi, [target1_array]
  mov   rdx, WEIGHTED_ROUND_ROBIN_IP
  mov   rcx, 2
  mov   r9, FALSE
  call  create_middleware
  cmp   rax, 0
  jl    .error

  mov   rdi, [proxy]
  mov   rsi, [target1_group]
  mov   rdx, rax
  call  add_middleware
  cmp   rax, 0
  jl    .error

  ; create middleware
  mov   rdi, proxy_middleware
  mov   rsi, [target2_array]
  mov   rdx, ROUND_ROBIN_IP
  mov   rcx, 1
  mov   r9, FALSE
  call  create_middleware
  cmp   rax, 0
  jl    .error

  mov   rdi, [proxy]
  mov   rsi, [target2_group]
  mov   rdx, rax
  call  add_middleware
  cmp   rax, 0
  jl    .error
    
  mov   rdi, [proxy]
  call  server_run

  mov   rdi, SUCCESS_CODE
  call  exit

.error:
  mov   rdi, FAILURE_CODE
  call  exit

section .data
  proxy dq 0

  target1_array dq 0
  target2_array dq 0

  target1_group dq 0
  target2_group dq 0

  target1_port equ 4000
  target2_port equ 4001
  target3_port equ 4002

  target1_weight equ 2
  target3_weight equ 3

  target1_id db "127.0.0.1", NULL_CHAR
  target2_id db "127.0.0.1", NULL_CHAR
  target3_id db "127.0.0.1", NULL_CHAR

  target1 dq 0
  target2 dq 0
  target3 dq 0

  target1_url db "/public", NULL_CHAR
  target2_url db "/internal", NULL_CHAR


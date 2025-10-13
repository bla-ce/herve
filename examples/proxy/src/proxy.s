global _start

%include "herve.inc"

section .text
_start:
  call  server_init
  cmp   rax, 0
  jl    .error

  mov   [proxy], rax

  ; create array of upstream servers
  mov   rdi, TARGET_COUNT
  mov   rsi, 8
  call  array_create
  cmp   rax, 0
  jl    .error

  mov   [target_arrays], rax

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
  mov   rdx, target2_weight
  call  proxy_target_create
  cmp   rax, 0
  jl    .error

  mov   [target2], rax

  mov   rdi, [target_arrays]
  mov   rsi, target1
  call  array_push
  cmp   rax, 0
  jl    .error

  mov   rdi, [target_arrays]
  mov   rsi, target2
  call  array_push
  cmp   rax, 0
  jl    .error

  ; create middleware
  mov   rdi, proxy_middleware
  mov   rsi, [target_arrays]
  mov   rdx, WEIGHTED_ROUND_ROBIN_IP
  mov   rcx, TARGET_COUNT
  mov   r9, FALSE
  call  create_middleware
  cmp   rax, 0
  jl    .error

  mov   rdi, [proxy]
  xor   rsi, rsi
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

  target_arrays dq 0

  target1_port equ 4000
  target2_port equ 4001

  target1_weight equ 2
  target2_weight equ 3

  target1_id db "127.0.0.1", NULL_CHAR
  target2_id db "127.0.0.1", NULL_CHAR

  target1 dq 0
  target2 dq 0

  TARGET_COUNT equ 2


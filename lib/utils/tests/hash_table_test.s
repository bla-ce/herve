global _start

%include "herve.inc"

section .data

key1  db "key1", NULL_CHAR
key2  db "kdy1", NULL_CHAR
key3  db "key3", NULL_CHAR
key4  db "key4", NULL_CHAR
key5  db "key5", NULL_CHAR

value1  db "value1", NULL_CHAR
value2  db "value2", NULL_CHAR
value3  db "value3", NULL_CHAR
value4  db "value4", NULL_CHAR
value5  db "value5", NULL_CHAR

hash_table dq 0

section .text

_start:
  mov   rdi, 6
  call  ht_create
  cmp   rax, 0
  jl    .error

  mov   [hash_table], rax

  mov   rdi, [hash_table]
  mov   rsi, key1
  mov   rdx, value1
  call  ht_insert
  cmp   rax, 0
  jl    .error

  mov   rdi, [hash_table]
  mov   rsi, key2
  mov   rdx, value2
  call  ht_insert
  cmp   rax, 0
  jl    .error

  mov   rdi, [hash_table]
  mov   rsi, key3
  mov   rdx, value3
  call  ht_insert
  cmp   rax, 0
  jl    .error

  mov   rdi, [hash_table]
  mov   rsi, key4
  mov   rdx, value4
  call  ht_insert
  cmp   rax, 0
  jl    .error

  mov   rdi, [hash_table]
  mov   rsi, key5
  mov   rdx, value5
  call  ht_insert
  cmp   rax, 0
  jl    .error

  mov   rdi, [hash_table]
  mov   rsi, key1
  call  ht_get
  cmp   rax, 0
  jl    .error

  mov   rdi, value1
  mov   rsi, rax
  call  strcmp
  cmp   rax, 0
  jl    .error
  cmp   rax, TRUE
  jne   .error

  mov   rdi, [hash_table]
  mov   rsi, key2
  call  ht_get
  cmp   rax, 0
  jl    .error

  mov   rdi, value2
  mov   rsi, rax
  call  strcmp
  cmp   rax, 0
  jl    .error
  cmp   rax, TRUE
  jne   .error

  mov   rdi, [hash_table]
  mov   rsi, key5
  mov   rdx, value4
  call  ht_insert
  cmp   rax, 0
  jl    .error

  mov   rdi, [hash_table]
  mov   rsi, key5
  call  ht_get
  cmp   rax, 0
  jl    .error

  mov   rdi, value4
  mov   rsi, rax
  call  strcmp
  cmp   rax, 0
  jl    .error
  cmp   rax, TRUE
  jne   .error

  mov   rdi, SUCCESS_CODE
  call  exit

.error:
  mov   rdi, FAILURE_CODE
  call  exit

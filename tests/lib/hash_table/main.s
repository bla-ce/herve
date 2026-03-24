global _start

%include "lib.inc"

section .data

key1  db "Host", NULL_CHAR
key2  db "User-Agent", NULL_CHAR
key3  db "Accept", NULL_CHAR
key4  db "Content-Type", NULL_CHAR
key5  db "Content-Length", NULL_CHAR

value1  db "localhost:1337", NULL_CHAR
value2  db "curl/7.88.1", NULL_CHAR
value3  db "*/*", NULL_CHAR
value4  db "application/x-www-form-urlencoded", NULL_CHAR
value5  db "36", NULL_CHAR

hash_table dq 0
keys_array dq 0

section .text

_start:
  mov   rdi, 5
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
  jle   .error

  mov   rdi, value1
  mov   rsi, rax
  call  assert_string_equal

  mov   rdi, [hash_table]
  mov   rsi, key2
  call  ht_get
  cmp   rax, 0
  jle   .error

  mov   rdi, value2
  mov   rsi, rax
  call  assert_string_equal

  mov   rdi, [hash_table]
  mov   rsi, key3
  call  ht_get
  cmp   rax, 0
  jle   .error

  mov   rdi, value3
  mov   rsi, rax
  call  assert_string_equal

  mov   rdi, [hash_table]
  mov   rsi, key4
  call  ht_get
  cmp   rax, 0
  jle   .error

  mov   rdi, value4
  mov   rsi, rax
  call  assert_string_equal

  mov   rdi, [hash_table]
  mov   rsi, key5
  call  ht_get
  cmp   rax, 0
  jle   .error

  mov   rdi, value5
  mov   rsi, rax
  call  assert_string_equal

.b1:
  mov   rdi, [hash_table]
  mov   rsi, key1
  mov   rdx, value2
  call  ht_insert
  cmp   rax, 0
  jl    .error

  mov   rdi, [hash_table]
  mov   rsi, key1
  call  ht_get
  cmp   rax, 0
  jle   .error

  mov   rdi, value2
  mov   rsi, rax
  call  assert_string_equal

  mov   rdi, [hash_table]
  mov   rsi, key1
  call  ht_del
  cmp   rax, 0
  jl    .error

  mov   rdi, [hash_table]
  mov   rsi, key1
  call  ht_get
  test  rax, rax
  jnz   .error

  mov   rdi, [hash_table]
  call  ht_free
  cmp   rax, 0
  jl    .error

  ; test ht get keys
  mov   rdi, 20
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
  call  ht_get_keys
  cmp   rax, 0
  jl    .error

  mov   [keys_array], rax

  ; get number of keys
  mov   rdi, rax
  call  array_get_length
  cmp   rax, 0
  jl    .error

  mov   r9, rax
  dec   r9

  ; print keys
.loop:
  mov   rdi, [keys_array]
  mov   rsi, r9
  call  array_get
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  println
  cmp   rax, 0
  jl    .error

  dec   r9
  jge   .loop
.loop_end:

  mov   rdi, [keys_array]
  call  array_free
  cmp   rax, 0
  jl    .error

  mov   rdi, [hash_table]
  call  ht_free
  cmp   rax, 0
  jl    .error

  mov   rdi, [mallocd]
  mov   rsi, [freed]
  call  assert_equal

  mov   rdi, SUCCESS_CODE
  call  exit

.error:
  mov   rdi, FAILURE_CODE
  call  exit

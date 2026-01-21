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
value5  dq "36", NULL_CHAR

hash_table dq 0

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
  cmp   qword [HT_ERR_MISSING_KEY], TRUE
  je    .error

  mov   rdi, value1
  mov   rsi, rax
  call  assert_string_equal

  mov   rdi, [hash_table]
  mov   rsi, key2
  call  ht_get
  cmp   qword [HT_ERR_MISSING_KEY], TRUE
  je    .error

  mov   rdi, value2
  mov   rsi, rax
  call  assert_string_equal

  mov   rdi, [hash_table]
  mov   rsi, key3
  call  ht_get
  cmp   qword [HT_ERR_MISSING_KEY], TRUE
  je    .error

  mov   rdi, value3
  mov   rsi, rax
  call  assert_string_equal

  mov   rdi, [hash_table]
  mov   rsi, key4
  call  ht_get
  cmp   qword [HT_ERR_MISSING_KEY], TRUE
  je    .error

  mov   rdi, value4
  mov   rsi, rax
  call  assert_string_equal

  mov   rdi, [hash_table]
  mov   rsi, key5
  call  ht_get
  cmp   qword [HT_ERR_MISSING_KEY], TRUE
  je    .error

  mov   rdi, value5
  mov   rsi, rax
  call  assert_string_equal

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

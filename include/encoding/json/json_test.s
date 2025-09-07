global _start

%include "herve.inc"
%include "os.inc"

section .text

_start:
  sub   rsp, 0x8

  call  json_create 
  cmp   rax, 0
  jl    .error

  mov   [rsp], rax

  mov   rdi, [rsp]
  mov   rsi, first_name_key
  mov   rdx, first_name_value
  call  json_insert_string
  cmp   rax, 0
  jl    .error
  
  mov   [rsp], rax

  mov   rdi, [rsp]
  mov   rsi, last_name_key
  mov   rdx, last_name_value
  call  json_insert_string
  cmp   rax, 0
  jl    .error
  
  mov   [rsp], rax

  mov   rdi, [rsp]
  mov   rsi, is_alive_key
  mov   rdx, TRUE
  call  json_insert_bool
  cmp   rax, 0
  jl    .error
  
  mov   [rsp], rax

  mov   rdi, [rsp]
  mov   rsi, is_working_key
  mov   rdx, FALSE
  call  json_insert_bool
  cmp   rax, 0
  jl    .error
  
  mov   [rsp], rax

  mov   rdi, [rsp]
  mov   rsi, age_key
  mov   rdx, 27
  call  json_insert_integer
  cmp   rax, 0
  jl    .error
  
  mov   [rsp], rax

  mov   rdi, [rsp]
  mov   rsi, spouse_key
  call  json_insert_null
  cmp   rax, 0
  jl    .error
  
  mov   [rsp], rax

  mov   rdi, [rsp]
  call  json_end
  cmp   rax, 0
  jl    .error

  mov   [rsp], rax

  mov   rdi, [rsp]
  call  println
  cmp   rax, 0
  jl    .error

  ; compare both strings
  mov   rdi, [rsp]
  mov   rsi, target_json
  call  strcmp
  cmp   rax, 0
  jl    .error

  cmp   rax, TRUE
  jne   .error

  mov   rdi, [rsp]
  call  json_free
  cmp   rax, 0
  jl    .error

  mov   rdi, SUCCESS_CODE
  call  exit

.error:
  mov   rdi, FAILURE_CODE
  call  exit

section .data
  first_name_key  db "first_name", NULL_CHAR
  last_name_key   db "last_name", NULL_CHAR
  is_alive_key    db "is_alive", NULL_CHAR
  is_working_key  db "is_working", NULL_CHAR
  spouse_key      db "spouse", NULL_CHAR
  age_key         db "age", NULL_CHAR

  first_name_value  db "John", NULL_CHAR
  last_name_value   db "Smith", NULL_CHAR

  target_json db '{'
              db '"first_name":"John",'
              db '"last_name":"Smith",'
              db '"is_alive":true,'
              db '"is_working":false,'
              db '"age":27,'
              ; db '"personal_income":87467.89,'
              ; db '"address":{'
              ; db '"street_address":"21 2nd Street",'
              ; db '"city":"New York",'
              ; db '"state":"NY",'
              ; db '"postal_code":"10021-3100"'
              ; db '},'
              ; db '"phone_numbers":['
              ; db '{'
              ; db '"type":"home",'
              ; db '"number":"212 555-1234"'
              ; db '},'
              ; db '{'
              ; db '"type":"office",'
              ; db '"number":"646 555-4567"'
              ; db '}'
              ; db '],'
              ; db '"children":['
              ; db '"Catherine",'
              ; db '"Thomas",'
              ; db '"Trevor"'
              ; db '],'
              db '"spouse":null'
              db '}', NULL_CHAR

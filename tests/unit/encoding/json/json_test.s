global _start

%include "lib.inc"

section .text

_start:
  sub   rsp, 0x30

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the json object
  ; [rsp+0x8]   -> pointer to the nested json object
  ; [rsp+0x10]  -> pointer to the children array
  ; [rsp+0x18]  -> pointer to the phone numbers array
  ; [rsp+0x20]  -> pointer to the first phone json object
  ; [rsp+0x28]  -> pointer to the second phone json object

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

  ; create second object
  call  json_create
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, [rsp+0x8]
  mov   rsi, street_address_key
  mov   rdx, street_address_value
  call  json_insert_string
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, [rsp+0x8]
  mov   rsi, city_key
  mov   rdx, city_value
  call  json_insert_string
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, [rsp+0x8]
  mov   rsi, state_key
  mov   rdx, state_value
  call  json_insert_string
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, [rsp+0x8]
  mov   rsi, postal_code_key
  mov   rdx, postal_code_value
  call  json_insert_string
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, [rsp+0x8]
  call  json_end
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; insert nested object
  mov   rdi, [rsp]
  mov   rsi, address_key
  mov   rdx, [rsp+0x8]
  call  json_insert_object
  cmp   rax, 0
  jl    .error

  mov   [rsp], rax

  ; free nested object
  mov   rdi, [rsp+0x8]
  call  json_free
  cmp   rax, 0
  jl    .error

  ; create array
  call  json_array_create
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  mov   rdi, [rsp+0x10]
  mov   rsi, children1_value
  call  json_array_insert_string
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  mov   rdi, [rsp+0x10]
  mov   rsi, children2_value
  call  json_array_insert_string
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  mov   rdi, [rsp+0x10]
  mov   rsi, children3_value
  call  json_array_insert_string
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; end array
  mov   rdi, [rsp+0x10]
  call  json_array_end
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; insert array
  mov   rdi, [rsp]
  mov   rsi, children_key
  mov   rdx, [rsp+0x10]
  call  json_insert_array
  cmp   rax, 0
  jl    .error

  mov   [rsp], rax

  ; free array
  mov   rdi, [rsp+0x10]
  call  json_array_free
  cmp   rax, 0
  jl    .error

  ; create phone number array
  call  json_array_create
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; create first phone number object
  call  json_create
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  mov   rdi, [rsp+0x20]
  mov   rsi, type_key
  mov   rdx, type_value1
  call  json_insert_string
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  mov   rdi, [rsp+0x20]
  mov   rsi, number_key
  mov   rdx, number_value1
  call  json_insert_string
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  mov   rdi, [rsp+0x20]
  mov   rsi, id_key
  mov   rdx, 0
  call  json_insert_integer
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  mov   rdi, [rsp+0x20]
  call  json_end
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  ; insert object
  mov   rdi, [rsp+0x18]
  mov   rsi, [rsp+0x20]
  call  json_array_insert_object
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; free object
  mov   rdi, [rsp+0x20]
  call  json_free
  cmp   rax, 0
  jl    .error

  ; create second phone number object
  call  json_create
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  mov   rdi, [rsp+0x20]
  mov   rsi, type_key
  mov   rdx, type_value2
  call  json_insert_string
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  mov   rdi, [rsp+0x20]
  mov   rsi, number_key
  mov   rdx, number_value2
  call  json_insert_string
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  mov   rdi, [rsp+0x20]
  mov   rsi, id_key
  mov   rdx, 1
  call  json_insert_integer
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  mov   rdi, [rsp+0x20]
  call  json_end
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  ; insert object
  mov   rdi, [rsp+0x18]
  mov   rsi, [rsp+0x20]
  call  json_array_insert_object
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; free object
  mov   rdi, [rsp+0x20]
  call  json_free
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x18]
  call  json_array_end
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; insert array
  mov   rdi, [rsp]
  mov   rsi, phone_numbers_key
  mov   rdx, [rsp+0x18]
  call  json_insert_array
  cmp   rax, 0
  jl    .error

  mov   [rsp], rax

  ; free array
  mov   rdi, [rsp+0x18]
  call  json_array_free
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, personal_income_key
  mov   rdx, qword [personal_income_value]
  mov   rcx, 2
  call  json_insert_float
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
  first_name_key      db "first_name", NULL_CHAR
  last_name_key       db "last_name", NULL_CHAR
  is_alive_key        db "is_alive", NULL_CHAR
  is_working_key      db "is_working", NULL_CHAR
  spouse_key          db "spouse", NULL_CHAR
  age_key             db "age", NULL_CHAR
  personal_income_key db "personal_income", NULL_CHAR
  address_key         db "address", NULL_CHAR
  street_address_key  db "street_address", NULL_CHAR
  city_key            db "city", NULL_CHAR
  state_key           db "state", NULL_CHAR
  postal_code_key     db "postal_code", NULL_CHAR
  children_key        db "children", NULL_CHAR
  phone_numbers_key   db "phone_numbers", NULL_CHAR
  type_key            db "type", NULL_CHAR
  number_key          db "number", NULL_CHAR
  id_key              db "id", NULL_CHAR

  first_name_value      db "John", NULL_CHAR
  last_name_value       db "Smith", NULL_CHAR
  street_address_value  db "21 2nd Street", NULL_CHAR
  city_value            db "New York", NULL_CHAR
  state_value           db "NY", NULL_CHAR
  postal_code_value     db "10021-3100", NULL_CHAR
  children1_value       db "Catherine", NULL_CHAR
  children2_value       db "Thomas", NULL_CHAR
  children3_value       db "Trevor", NULL_CHAR
  type_value1           db "home", NULL_CHAR
  number_value1         db "212 555-1234", NULL_CHAR
  type_value2           db "office", NULL_CHAR
  number_value2         db "646 555-4567", NULL_CHAR
  personal_income_value dq 87467.89

  target_json db '{'
              db '"first_name":"John",'
              db '"last_name":"Smith",'
              db '"is_alive":true,'
              db '"is_working":false,'
              db '"age":27,'
              db '"address":{'
              db '"street_address":"21 2nd Street",'
              db '"city":"New York",'
              db '"state":"NY",'
              db '"postal_code":"10021-3100"'
              db '},'
              db '"children":['
              db '"Catherine",'
              db '"Thomas",'
              db '"Trevor"'
              db '],'
              db '"phone_numbers":['
              db '{'
              db '"type":"home",'
              db '"number":"212 555-1234",'
              db '"id":0',
              db '},'
              db '{'
              db '"type":"office",'
              db '"number":"646 555-4567",'
              db '"id":1',
              db '}'
              db '],'
              db '"personal_income":87467.89,'
              db '"spouse":null'
              db '}', NULL_CHAR

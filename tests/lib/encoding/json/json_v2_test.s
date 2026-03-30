global _start

%include "lib.inc"

section .text

_start:
  call  json_create_object
  cmp   rax, 0
  jl    .error

  mov   [person_json], rax

  mov   rdi, first_name_value
  call  json_create_string
  cmp   rax, 0
  jl    .error

  mov   rdi, [person_json]
  mov   rsi, rax
  mov   rdx, first_name_key
  call  json_add_entry_to_object
  cmp   rax, 0
  jl    .error

  mov   rdi, last_name_value
  call  json_create_string
  cmp   rax, 0
  jl    .error

  mov   rdi, [person_json]
  mov   rsi, rax
  mov   rdx, last_name_key
  call  json_add_entry_to_object
  cmp   rax, 0
  jl    .error

  mov   rdi, [json_string]
  call  free
  cmp   rax, 0
  jl    .error

  mov   rdi, [freed]
  mov   rsi, [malloc]
  call  assert_equal

  mov   rdi, SUCCESS_CODE
  call  exit

.error:
  mov   rdi, FAILURE_CODE
  call  exit

section .data
  person_json dq 0

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

  target_json db  '{'
              db    '"first_name":"John",'
              db    '"last_name":"Smith",'
              db    '"is_alive":true,'
              db    '"is_working":false,'
              db    '"age":27,'
              db    '"address":{'
              db      '"street_address":"21 2nd Street",'
              db      '"city":"New York",'
              db      '"state":"NY",'
              db      '"postal_code":"10021-3100"'
              db    '},'
              db    '"children":['
              db      '"Catherine",'
              db      '"Thomas",'
              db      '"Trevor"'
              db    '],'
              db    '"phone_numbers":['
              db      '{'
              db        '"type":"home",'
              db        '"number":"212 555-1234",'
              db        '"id":0',
              db      '},'
              db      '{'
              db        '"type":"office",'
              db        '"number":"646 555-4567",'
              db        '"id":1',
              db      '}'
              db    '],'
              db    '"personal_income":87467.89,'
              db    '"spouse":null'
              db  '}', NULL_CHAR


  JSON_KV_DELIM   db ":", NULL_CHAR

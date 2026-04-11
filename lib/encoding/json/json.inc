section .data

JSON_BEGIN      db "{", NULL_CHAR
JSON_END        db "}", NULL_CHAR

JSON_ARRAY_BEGIN  db "[", NULL_CHAR
JSON_ARRAY_END    db "]", NULL_CHAR

JSON_STR_QUOTES db DOUBLE_QUOTES, NULL_CHAR
JSON_KV_DELIM   db ":", NULL_CHAR
JSON_KEYS_DELIM db ",", NULL_CHAR

JSON_BOOL_FALSE db "false", NULL_CHAR
JSON_BOOL_TRUE  db "true", NULL_CHAR

JSON_NULL db "null", NULL_CHAR

JSON_FLOAT_DOT db ".", NULL_CHAR

section .text

; initalizes a json object
; @return rax: pointer to the json object
json_create:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp] -> pointer to the json object

  call  boeuf_init
  cmp   rax, 0
  jl    .error

  mov   [rsp], rax

  mov   rdi, rax
  mov   rsi, JSON_BEGIN
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; inserts the key of the kv pair
; @param  rdi: pointer to the json object
; @param  rsi: pointer to the key
; @return rax: return code
json_insert_key:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> pointer to the json object
  ; [rsp+0x8]   -> pointer to the key
  ; [rsp+0x10]  -> pointer to the boeuf data

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  call  boeuf_get_data
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  mov   rdi, [rsp]
  call  _boeuf_get_len
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x10]
  add   rdi, rax
  dec   rdi

  cmp   byte [rdi], OPENING_BRACKET
  je    .no_comma

  cmp   byte [rdi], OPENING_BRACE
  je    .no_comma

  ; insert comma
  mov   rdi, [rsp]
  mov   rsi, JSON_KEYS_DELIM
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

.no_comma:
  ; insert double quote
  mov   rdi, [rsp]
  mov   rsi, JSON_STR_QUOTES
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  ; insert string
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  ; insert double quote
  mov   rdi, [rsp]
  mov   rsi, JSON_STR_QUOTES
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  ; insert delim
  mov   rdi, [rsp]
  mov   rsi, JSON_KV_DELIM
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; inserts an integer into the json object
; @param  rdi: pointer to the json object
; @param  rsi: pointer to the key
; @param  rdx: integer
; @return rax: return code
json_insert_integer:
  sub   rsp, 0x18
  sub   rsp, SIGNED_LONG_LONG_MAX_STR_SIZE

  ; STACK USAGE
  ; [rsp]       -> pointer to the json object
  ; [rsp+0x8]   -> pointer to the key
  ; [rsp+0x10]  -> integer
  ; [rsp+0x18]  -> tmp string to hold the number

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  ; insert key
  call  json_insert_key
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x10]
  lea   rsi, [rsp+0x18]
  mov   rdx, SIGNED_LONG_LONG_MAX_STR_SIZE
  call  itoa
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, rax
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, SIGNED_LONG_LONG_MAX_STR_SIZE
  add   rsp, 0x18
  ret

; inserts a json object into the json object
; @param  rdi: pointer to the json object
; @param  rsi: pointer to the key
; @param  rdx: pointer to the json object to insert
; @return rax: return code
json_insert_object:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> pointer to the json object
  ; [rsp+0x8]   -> pointer to the key
  ; [rsp+0x10]  -> pointer to the json object to insert

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  test  rdx, rdx
  jz    .error

  ; insert key
  call  json_insert_key
  cmp   rax, 0
  jl    .error

  ; insert json object
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x10]
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; inserts a string into the json object
; @param  rdi: pointer to the json object
; @param  rsi: pointer to the key
; @param  rdx: pointer to the value
; @return rax: return code
json_insert_string:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> pointer to the json object
  ; [rsp+0x8]   -> pointer to the key
  ; [rsp+0x10]  -> pointer to the value

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  test  rdx, rdx
  jz    .error

  ; insert key
  call  json_insert_key
  cmp   rax, 0
  jl    .error

  ; insert double quote
  mov   rdi, [rsp]
  mov   rsi, JSON_STR_QUOTES
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  ; insert second string
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x10]
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  ; insert double quote
  mov   rdi, [rsp]
  mov   rsi, JSON_STR_QUOTES
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; inserts a boolean into the json object
; @param  rdi: pointer to the json object
; @param  rsi: pointer to the key
; @param  rdx: TRUE (1) | FALSE (0)
; @return rax: return code
json_insert_bool:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> pointer to the json object
  ; [rsp+0x8]   -> pointer to the key
  ; [rsp+0x10]  -> TRUE (1) | FALSE (0)

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  ; insert key
  call  json_insert_key
  cmp   rax, 0
  jl    .error

  cmp   qword [rsp+0x10], FALSE
  je    .insert_false

.insert_true:
  mov   rdi, [rsp]
  mov   rsi, JSON_BOOL_TRUE
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.insert_false:
  mov   rdi, [rsp]
  mov   rsi, JSON_BOOL_FALSE
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  ; return
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; inserts a float value into the json object
; @param  rdi: pointer to the json object
; @param  rsi: pointer to the key
; @param  rdx: float value
; @param  rcx: precision
; @return rax: return code
json_insert_float:
  sub   rsp, 0x28
  sub   rsp, SIGNED_LONG_LONG_MAX_STR_SIZE

  ; STACK USAGE
  ; [rsp]       -> pointer to the json object
  ; [rsp+0x8]   -> pointer to the key
  ; [rsp+0x10]  -> float value
  ; [rsp+0x18]  -> precision
  ; [rsp+0x20]  -> power of ten to get remainder
  ; [rsp+0x28]  -> tmp string to hold the number

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   [rsp+0x18], rcx

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  ; insert key
  call  json_insert_key
  cmp   rax, 0
  jl    .error

  ; insert integer part
  cvttsd2si rdi, [rsp+0x10]
  lea   rsi, [rsp+0x28]
  mov   rdx, SIGNED_LONG_LONG_MAX_STR_SIZE
  call  itoa
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, rax
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  ; insert dot
  mov   rdi, [rsp]
  mov   rsi, JSON_FLOAT_DOT
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  ; get decimal part
  mov   rax, 1
.loop:
  cmp   qword [rsp+0x18], 0
  je    .loop_end

  xor   rdx, rdx
  mov   rbx, 10
  mul   rbx

  dec   qword [rsp+0x18]

  jmp   .loop

.loop_end:

  mov   [rsp+0x20], rax

  movsd xmm0, [rsp+0x10]
  movsd xmm1, [rsp+0x20]
  mulsd xmm0, xmm1

  xor   rdx, rdx
  movq  rax, xmm0
  mov   rbx, [rsp+0x20]
  div   rbx

  mov   rdi, rdx
  lea   rsi, [rsp+0x28]
  mov   rdx, SIGNED_LONG_LONG_MAX_STR_SIZE
  call  itoa
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, rax
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, SIGNED_LONG_LONG_MAX_STR_SIZE
  add   rsp, 0x28
  ret

; inserts a null value into the json object
; @param  rdi: pointer to the json object
; @param  rsi: pointer to the key
; @return rax: return code
json_insert_null:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]       -> pointer to the json object
  ; [rsp+0x8]   -> pointer to the key

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  ; insert key
  call  json_insert_key
  cmp   rax, 0
  jl    .error

  ; insert double quote
  mov   rdi, [rsp]
  mov   rsi, JSON_NULL
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  ; return
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; initialise a json array
; @return rax: pointer to the json array
json_array_create:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp] -> pointer to the json object

  call  boeuf_init
  cmp   rax, 0
  jl    .error

  mov   [rsp], rax

  mov   rdi, rax
  mov   rsi, JSON_ARRAY_BEGIN
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; inserts a float value into the json array
; @param  rdi: pointer to the json array
; @param  rsi: pointer to the key
; @param  rdx: float value
; @param  rcx: precision
; @return rax: return code
json_array_insert_float:
  sub   rsp, 0x20
  sub   rsp, SIGNED_LONG_LONG_MAX_STR_SIZE

  ; STACK USAGE
  ; [rsp]       -> pointer to the json array
  ; [rsp+0x8]  -> float value
  ; [rsp+0x10]  -> precision
  ; [rsp+0x18]  -> power of ten to get remainder
  ; [rsp+0x20]  -> tmp string to hold the number

  mov   [rsp], rdi
  mov   [rsp+0x8], rdx
  mov   [rsp+0x10], rcx

  test  rdi, rdi
  jz    .error

  ; insert integer part
  cvttsd2si rdi, [rsp+0x8]
  lea   rsi, [rsp+0x20]
  mov   rdx, SIGNED_LONG_LONG_MAX_STR_SIZE
  call  itoa
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, rax
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  ; insert dot
  mov   rdi, [rsp]
  mov   rsi, JSON_FLOAT_DOT
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  ; get decimal part
  mov   rax, 1
.loop:
  cmp   qword [rsp+0x10], 0
  je    .loop_end

  xor   rdx, rdx
  mov   rbx, 10
  mul   rbx

  dec   qword [rsp+0x10]

  jmp   .loop

.loop_end:

  mov   [rsp+0x18], rax

  movsd xmm0, [rsp+0x8]
  movsd xmm1, [rsp+0x18]
  mulsd xmm0, xmm1

  xor   rdx, rdx
  movq  rax, xmm0
  mov   rbx, [rsp+0x18]
  div   rbx

  mov   rdi, rdx
  lea   rsi, [rsp+0x20]
  mov   rdx, SIGNED_LONG_LONG_MAX_STR_SIZE
  call  itoa
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, rax
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  ; add comma
  mov   rdi, [rsp]
  mov   rsi, JSON_KEYS_DELIM
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, SIGNED_LONG_LONG_MAX_STR_SIZE
  add   rsp, 0x20
  ret

; inserts an integer into the json array
; @param  rdi: pointer to the json array
; @param  rsi: integer
; @return rax: return code
json_array_insert_integer:
  sub   rsp, 0x10
  sub   rsp, SIGNED_LONG_LONG_MAX_STR_SIZE

  ; STACK USAGE
  ; [rsp]       -> pointer to the json array
  ; [rsp+0x8]   -> integer
  ; [rsp+0x10]  -> tmp string for holding the number

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  test  rdi, rdi
  jz    .error

  mov   rdi, [rsp+0x8]
  lea   rsi, [rsp+0x10]
  mov   rdx, SIGNED_LONG_LONG_MAX_STR_SIZE
  call  itoa
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, rax
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  ; add comma
  mov   rdi, [rsp]
  mov   rsi, JSON_KEYS_DELIM
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, SIGNED_LONG_LONG_MAX_STR_SIZE
  add   rsp, 0x10
  ret

; inserts a json object into the json array
; @param  rdi: pointer to the json array
; @param  rsi: pointer to the json object to insert
; @return rax: return code
json_array_insert_object:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> pointer to the json object
  ; [rsp+0x8] -> pointer to the json object to insert

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  ; insert json object
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  ; add comma
  mov   rdi, [rsp]
  mov   rsi, JSON_KEYS_DELIM
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; inserts a string into the json object
; @param  rdi: pointer to the json object
; @param  rsi: pointer to the value
; @return rax: return code
json_array_insert_string:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> pointer to the json object
  ; [rsp+0x8] -> pointer to the value

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  ; insert double quote
  mov   rdi, [rsp]
  mov   rsi, JSON_STR_QUOTES
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  ; insert second string
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  ; insert double quote
  mov   rdi, [rsp]
  mov   rsi, JSON_STR_QUOTES
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  ; add comma
  mov   rdi, [rsp]
  mov   rsi, JSON_KEYS_DELIM
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; inserts a boolean into the json object
; @param  rdi: pointer to the json object
; @param  rsi: TRUE (1) | FALSE (0)
; @return rax: return code
json_array_insert_bool:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> pointer to the json object
  ; [rsp+0x8] -> TRUE (1) | FALSE (0)

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  cmp   rdi, 0
  jle   .error

  cmp   qword [rsp+0x8], FALSE
  je    .insert_false

.insert_true:
  mov   rdi, [rsp]
  mov   rsi, JSON_BOOL_TRUE
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  jmp   .insert_comma

.insert_false:
  mov   rdi, [rsp]
  mov   rsi, JSON_BOOL_FALSE
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

.insert_comma:
  mov   rdi, [rsp]
  mov   rsi, JSON_KEYS_DELIM
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; inserts a null value into the json object
; @param  rdi: pointer to the json object
; @return rax: return code
json_array_insert_null:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp] -> pointer to the json object

  mov   [rsp], rdi

  test  rdi, rdi
  jz    .error

  ; insert double quote
  mov   rdi, [rsp]
  mov   rsi, JSON_NULL
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  ; insert comma
  mov   rdi, [rsp]
  mov   rsi, JSON_KEYS_DELIM
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; inserts a json array into the json object
; @param  rdi: pointer to the json object
; @param  rsi: pointer to the key
; @param  rdx: pointer to the array
; @return rax: return code
json_insert_array:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> pointer to the json object
  ; [rsp+0x8]   -> pointer to the key
  ; [rsp+0x10]  -> pointer to the array

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  test  rdx, rdx
  jz    .error

  ; insert key
  call  json_insert_key
  cmp   rax, 0
  jl    .error

  ; insert json array
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x10]
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; ends the json array
; @param  rdi: pointer to the json array
; @return rax: return code
json_array_end:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> pointer to the json array
  ; [rsp+0x8] -> pointer to boeuf data

  mov   [rsp], rdi

  test  rdi, rdi
  jz    .error

  call  boeuf_get_data
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, [rsp]
  call  _boeuf_get_len
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  add   rdi, rax
  dec   rdi

  cmp   byte [rdi], OPENING_BRACKET
  jne   .replace

  ; append closing
  mov   rdi, [rsp]
  mov   rsi, JSON_ARRAY_END
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.replace:
  ; replace comma with ']'
  mov   rax, CLOSING_BRACKET
  stosb

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; frees the json array
; @param  rdi: pointer to the json array
; @return rax: return code
json_array_free:
  call  boeuf_free
  ret

; ends the json object
; @param  rdi: pointer to the json object
; @return rax: return code
json_end:
  mov   rsi, JSON_END
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the stringified json
; @param  rdi: pointer to the boeuf struct
; @return rax: str(json)
json_to_str:
  test  rdi, rdi
  jz    .error

  call  boeuf_to_str

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; frees the json object
; @param  rdi: pointer to the json object
; @return rax: return code
json_free:
  call  boeuf_free
  ret

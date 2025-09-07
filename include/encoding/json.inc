section .data

JSON_BEGIN      db "{", NULL_CHAR
JSON_END        db "}", NULL_CHAR
JSON_STR_QUOTES db DOUBLE_QUOTES, NULL_CHAR
JSON_KV_DELIM   db ":", NULL_CHAR
JSON_KEYS_DELIM db COMMA, NULL_CHAR

section .text

; initalizes a json object
; @return rax: pointer to the json object
json_create:
  sub   rsp, 0x8

  ; *** STACK USAGE *** ;
  ; [rsp] -> pointer to the json object

  mov   rdi, JSON_BEGIN 
  call  boeuf_create
  cmp   rax, 0
  jl    .error

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; inserts a string into the json object
; @param  rdi: pointer to the json object
; @param  rsi: pointer to the key
; @param  rdx: pointer to the value
; @return rax: pointer to the updated json object
json_insert_string:
  sub   rsp, 0x18

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the json object
  ; [rsp+0x8]   -> pointer to the key
  ; [rsp+0x10]  -> pointer to the value

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  cmp   rdx, 0
  jle   .error

  ; check the character before if '{' or '[', don't add comma
  mov   rdi, [rsp]
  call  strlen
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  add   rdi, rax
  dec   rdi

  cmp   byte [rdi], OPENING_BRACKET
  je    .no_comma

  cmp   byte [rdi], OPENING_BRACE
  je    .no_comma

  ; insert comma
  mov   rdi, [rsp]
  mov   rsi, JSON_KEYS_DELIM
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  mov   [rsp], rax

.no_comma:
  ; insert double quote
  mov   rdi, [rsp]
  mov   rsi, JSON_STR_QUOTES
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  ; insert string
  mov   rdi, rax
  mov   rsi, [rsp+0x8]
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  ; insert double quote
  mov   rdi, rax
  mov   rsi, JSON_STR_QUOTES
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  ; insert delim
  mov   rdi, rax
  mov   rsi, JSON_KV_DELIM
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  ; insert double quote
  mov   rdi, rax
  mov   rsi, JSON_STR_QUOTES
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  ; insert second string
  mov   rdi, rax
  mov   rsi, [rsp+0x10]
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  ; insert double quote
  mov   rdi, rax
  mov   rsi, JSON_STR_QUOTES
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  ; return
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; ends the json object
; @param  rdi: pointer to the json object
; @return rax: pointer to the json object
json_end:
  mov   rsi, JSON_END
  call  boeuf_append
  cmp   rax, 0
  jl    .error

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


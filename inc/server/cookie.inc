section .bss
cookie_struct:
  cookie_name      resq 1
  cookie_value     resq 1
  cookie_max_age   resq 1
  cookie_http_only resq 1
  cookie_secure    resq 1
  cookie_domain    resq 1
cookie_struct_end:

section .data
COOKIE_STRUCT_LEN equ cookie_struct_end - cookie_struct

; cookie struct offsets
COOKIE_OFF_NAME       equ 0x0
COOKIE_OFF_VALUE      equ COOKIE_OFF_NAME + 0x8
COOKIE_OFF_MAX_AGE    equ COOKIE_OFF_VALUE + 0x8
COOKIE_OFF_HTTP_ONLY  equ COOKIE_OFF_MAX_AGE + 0x8
COOKIE_OFF_SECURE     equ COOKIE_OFF_HTTP_ONLY + 0x8
COOKIE_OFF_DOMAIN     equ COOKIE_OFF_SECURE + 0x8

COOKIE_MAX_SIZE equ 0x1000

ATTRIBUTE_DELIM db "; ", NULL_CHAR

; cookie attributes
MAX_AGE_ATTR    db "Max-Age", NULL_CHAR
SECURE_ATTR     db "Secure", NULL_CHAR
HTTP_ONLY_ATTR  db "HttpOnly", NULL_CHAR
DOMAIN_ATTR     db "Domain", NULL_CHAR

; token_table: each byte is 0 or 1 indicating invalid or valid token char
;              for ASCII code = index in [0..127]
; 
;  0 => invalid token character
;  1 => valid token character
token_table:
  db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  db 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
  db 0,1,0,1,1,1,1,1,0,0,1,1,0,1,1,0
  db 1,1,1,1,1,1,1,1,1,1,0,0,0,0,0,0
  db 0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
  db 1,1,1,1,1,1,1,1,1,1,1,0,0,0,1,1
  db 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
  db 1,1,1,1,1,1,1,1,1,1,1,0,1,0,1,0
token_table_end:

section .text
; adds a Set-Cookie header to the response
; @param  rdi: pointer to the context struct
; @param  rsi: pointer to the cookie struct
; @return rax: return code
set_cookie:
  sub   rsp, 0x18

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; *** STACK USAGE *** ; 
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> pointer to the cookie struct
  ; [rsp+0x10]  -> pointer to the serialized cookie struct 

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  mov   rdi, rsi
  call  serialize_cookie
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  mov   rdi, [rsp]
  call  get_ctx_response
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  get_response_headers
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  lea   rsi, [SET_COOKIE_HEADER]
  mov   rdx, [rsp+0x10]
  call  set_header
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret


; checks whether or not the key of the cookie is valid
; @param  rdi: pointer to the key
; @return rax: (bool) is valid
is_cookie_key_valid:
  sub   rsp, 0x10

  cmp   rdi, 0
  jl    .false

  ; *** STACK USAGE *** ;
  ; [rsp]     -> pointer to the key
  ; [rsp+0x8] -> key length

  mov   [rsp], rdi

  ; get length of the key
  call  strlen
  cmp   rax, 0
  jle   .false

  mov   qword [rsp+0x8], rax

  mov   rdi, [rsp]
  xor   r9, r9

.loop:
  cmp   r9, qword [rsp+0x8]
  jge   .end_loop

  xor   rax, rax
  mov   al, byte [rdi+r9]
  movzx eax, byte [token_table + rax]
  cmp   eax, FALSE
  je    .false

  inc   r9
  jmp   .loop
.end_loop:

.true:
  mov   rax, TRUE
  jmp   .return

.false:
  mov   rax, FALSE

.return:
  add   rsp, 0x10
  ret

; creates cookie struct and returns a pointer to it
; user is responsible of freeing the struct
; @param  rdi:  pointer to the name
; @param  rsi:  pointer to the value
; @param  rdx:  max age
; @param  rcx:  http_only
; @param  r8:   secure
; @param  r9:   domain
; @return rax: pointer to the cookie struct
create_cookie:
  sub   rsp, 0x38

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the name
  ; [rsp+0x8]   -> pointer to the value
  ; [rsp+0x10]  -> max age of the cookie
  ; [rsp+0x18]  -> http_only attribute
  ; [rsp+0x20]  -> secure attribute
  ; [rsp+0x28]  -> domain attribute
  ; [rsp+0x30]  -> pointer to the cookie struct

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   [rsp+0x18], rcx
  mov   [rsp+0x20], r8
  mov   [rsp+0x28], r9

  ; make sure cookie name is valid
  call  is_cookie_key_valid
  cmp   rax, FALSE
  je    .error

  mov   rdi, COOKIE_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x30], rax

  mov   rdi, [rsp]
  mov   [rax+COOKIE_OFF_NAME], rdi

  mov   rdi, [rsp+0x8]
  mov   [rax+COOKIE_OFF_VALUE], rdi

  mov   rdi, [rsp+0x10]
  mov   [rax+COOKIE_OFF_MAX_AGE], rdi

  mov   rdi, [rsp+0x18]
  mov   [rax+COOKIE_OFF_HTTP_ONLY], rdi

  mov   rdi, [rsp+0x20]
  mov   [rax+COOKIE_OFF_SECURE], rdi

  mov   rdi, [rsp+0x28]
  mov   [rax+COOKIE_OFF_DOMAIN], rdi

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x38
  ret

; returns the serialization of the cookie
; @param  rdi: pointer to the cookie struct
; @return rax: pointer to the serialized cookie
serialize_cookie:
  sub   rsp, 0x18
  sub   rsp, SIGNED_LONG_LONG_MAX_STR_SIZE

  cmp   rdi, 0
  jle   .error

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the cookie struct
  ; [rsp+0x8]   -> pointer to the serialized cookie
  ; [rsp+0x10]  -> '=' string to append
  ; [rsp+0x18]  -> max age string

  mov   [rsp], rdi

  ; get cookie name
  mov   rax, [rdi+COOKIE_OFF_NAME]

  ; create boeuf buffer
  mov   rdi, rax
  call  boeuf_create
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; add equal
  mov   byte [rsp+0x10], EQUAL_SIGN
  mov   byte [rsp+0x10+1], NULL_CHAR

  mov   rdi, [rsp+0x8]
  lea   rsi, [rsp+0x10]
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; append cookie value
  mov   rdi, [rsp]
  mov   rsi, [rdi+COOKIE_OFF_VALUE]
  mov   rdi, rax
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; add cookie delimiter
  mov   rdi, rax
  mov   rsi, ATTRIBUTE_DELIM
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, [rsp]
  mov   rsi, [rdi+COOKIE_OFF_DOMAIN]
  cmp   rsi, 0
  je    .skip_domain

.b1:
  ; append domain key
  mov   rdi, [rsp+0x8]
  mov   rsi, DOMAIN_ATTR
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  ; add equal
  mov   rdi, rax
  lea   rsi, [rsp+0x10]
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; add domain value
  mov   rdi, [rsp]
  mov   rsi, [rdi+COOKIE_OFF_DOMAIN]
  mov   rdi, [rsp+0x8]
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

.skip_domain:
  ; get max age attribute
  mov   rdi, [rsp]
  mov   rax, qword [rdi+COOKIE_OFF_MAX_AGE]
  cmp   rax, 0
  je    .skip_max_age

  ; append delimiter
  mov   rdi, [rsp+0x8]
  mov   rsi, ATTRIBUTE_DELIM
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  ; append max age key
  mov   rdi, rax
  mov   rsi, MAX_AGE_ATTR
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  ; append equal
  mov   rdi, rax
  lea   rsi, [rsp+0x10]
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; copy max age string
  mov   rsi, [rsp]
  mov   rdi, qword [rsi+COOKIE_OFF_MAX_AGE]
  lea   rsi, [rsp+0x18]
  mov   rdx, SIGNED_LONG_LONG_MAX_STR_SIZE
  call  to_string
  cmp   rax, 0
  jl    .error

  ; append max age attribute
  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

.skip_max_age:
  ; http_only attribute
  mov   rdi, [rsp]
  mov   rax, qword [rdi+COOKIE_OFF_HTTP_ONLY]
  cmp   rax, 0
  je    .skip_http_only

  mov   rdi, [rsp+0x8]
  mov   rsi, ATTRIBUTE_DELIM
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  ; c
  mov   rdi, rax
  mov   rsi, HTTP_ONLY_ATTR
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

.skip_http_only:
  ; secure attribute
  mov   rdi, [rsp]
  mov   rax, qword [rdi+COOKIE_OFF_SECURE]
  cmp   rax, 0
  je    .skip_secure

  ; add '; '
  mov   rdi, [rsp+0x8]
  mov   rsi, ATTRIBUTE_DELIM
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  ; add secure
  mov   rdi, rax
  mov   rsi, SECURE_ATTR
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

.skip_secure:
  mov   rax, [rsp+0x8]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, SIGNED_LONG_LONG_MAX_STR_SIZE
  add   rsp, 0x18
  ret


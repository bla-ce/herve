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

; cookie attributes
MAX_AGE_ATTR      db "Max-Age", NULL_CHAR
MAX_AGE_ATTR_LEN  equ $ - MAX_AGE_ATTR - 1  ; remove null_char

SECURE_ATTR     db "Secure", NULL_CHAR
SECURE_ATTR_LEN equ $ - SECURE_ATTR - 1  ; remove null_char

HTTP_ONLY_ATTR      db "HttpOnly", NULL_CHAR
HTTP_ONLY_ATTR_LEN  equ $ - HTTP_ONLY_ATTR - 1  ; remove null_char

DOMAIN_ATTR     db "Domain", NULL_CHAR
DOMAIN_ATTR_LEN equ $ - DOMAIN_ATTR - 1

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
  sub   rsp, 0x68
  sub   rsp, SIGNED_LONG_LONG_MAX_STR_SIZE
  sub   rsp, COOKIE_MAX_SIZE

  cmp   rdi, 0
  jle   .error

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the cookie struct
  ; [rsp+0x8]   -> pointer to the cookie name
  ; [rsp+0x10]  -> pointer to the cookie value
  ; [rsp+0x18]  -> max age atribute
  ; [rsp+0x20]  -> http_only attribute
  ; [rsp+0x28]  -> secure attribute
  ; [rsp+0x30]  -> domain attribute
  ; [rsp+0x38]  -> cookie name length
  ; [rsp+0x40]  -> cookie value length
  ; [rsp+0x48]  -> serialized cookie length
  ; [rsp+0x50]  -> pointer to mallocd cookie string
  ; [rsp+0x58]  -> dynamic pointer cookie string
  ; [rsp+0x60]  -> max age string
  ; [rsp+0x60+SIGNED_LONG_LONG_MAX_STR_SIZE] -> serialized cookie

  mov   [rsp], rdi

  ; get cookie name
  mov   rax, [rdi+COOKIE_OFF_NAME]
  mov   [rsp+0x8], rax

  ; get cookie value
  mov   rax, [rdi+COOKIE_OFF_VALUE]
  mov   [rsp+0x10], rax

  ; get max age attribute
  mov   rax, qword [rdi+COOKIE_OFF_MAX_AGE]
  mov   qword [rsp+0x18], rax

  ; get http only attribute
  mov   rax, qword [rdi+COOKIE_OFF_HTTP_ONLY]
  mov   qword [rsp+0x20], rax

  ; get secure attribute
  mov   rax, qword [rdi+COOKIE_OFF_SECURE]
  mov   qword [rsp+0x28], rax

  ; get domain attribute
  mov   rax, qword [rdi+COOKIE_OFF_DOMAIN]
  mov   qword [rsp+0x30], rax

  ; get cookie name length
  mov   rdi, [rsp+0x8]
  call  strlen
  cmp   rax, 0
  jl    .error

  mov   qword [rsp+0x38], rax

  ; get cookie value length
  mov   rdi, [rsp+0x10]
  call  strlen
  cmp   rax, 0
  jl    .error

  mov   qword [rsp+0x40], rax

  ; copy name
  lea   rdi, [rsp+0x60+SIGNED_LONG_LONG_MAX_STR_SIZE]
  mov   rsi, [rsp+0x8]
  mov   rcx, qword [rsp+0x38]
  rep   movsb

  ; add equal sign
  mov   rax, EQUAL_SIGN
  stosb

  ; copy value
  mov   rsi, [rsp+0x10] 
  mov   rcx, qword [rsp+0x40]
  rep   movsb

  cmp   qword [rsp+0x18], 0
  je    .skip_max_age

  ; copy '; '
  mov   rax, SEMI_COLON
  stosb

  mov   rax, SPACE
  stosb

  ; copy attribute name
  lea   rsi, [MAX_AGE_ATTR]
  mov   rcx, MAX_AGE_ATTR_LEN
  rep   movsb

  ; copy EQUAL SIGN
  mov   rax, EQUAL_SIGN
  stosb

  ; save rdi position
  mov   [rsp+0x58], rdi

  ; copy max age string
  mov   rdi, qword [rsp+0x18]
  lea   rsi, [rsp+0x60]
  mov   rdx, SIGNED_LONG_LONG_MAX_STR_SIZE
  call  to_string
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x58]
  mov   rsi, rax
  call  strcpy
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   [rsp+0x58], rdi

.skip_max_age:
  ; http_only attribute
  cmp   qword [rsp+0x20], 0
  je    .skip_http_only

  ; add '; '
  mov   rax, SEMI_COLON
  stosb

  mov   rax, SPACE
  stosb

  lea   rsi, [HTTP_ONLY_ATTR]
  mov   rcx, HTTP_ONLY_ATTR_LEN
  rep   movsb

.skip_http_only:
  ; secure attribute
  cmp   qword [rsp+0x28], 0
  je    .skip_secure

  ; add '; '
  mov   rax, SEMI_COLON
  stosb

  mov   rax, SPACE
  stosb

  lea   rsi, [SECURE_ATTR]
  mov   rcx, SECURE_ATTR_LEN
  rep   movsb

.skip_secure:
  ; domain attribute
  cmp   qword [rsp+0x30], 0
  je    .skip_domain

  ; add '; '
  mov   rax, SEMI_COLON
  stosb

  mov   rax, SPACE
  stosb

  ; copy domain key
  lea   rsi, [DOMAIN_ATTR]
  mov   rcx, DOMAIN_ATTR_LEN
  rep   movsb

  mov   rax, EQUAL_SIGN
  stosb

  ; save current position
  mov   [rsp+0x58], rdi

  ; get length of domain value
  mov   rdi, [rsp+0x30]
  call  strlen
  cmp   rax, 0
  jl    .error

  mov   rcx, rax
  mov   rdi, [rsp+0x58]
  mov   rsi, [rsp+0x30]
  rep   movsb

.skip_domain:
  ; add null char
  mov   rax, NULL_CHAR
  stosb

  ; get length of serialized cookie
  lea   rdi, [rsp+0x60+SIGNED_LONG_LONG_MAX_STR_SIZE]
  call  strlen
  cmp   rax, 0
  jl    .error

  mov   qword [rsp+0x48], rax

  mov   rdi, rax
  inc   rdi       ; add one for null_char
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x50], rax

  mov   rdi, rax
  lea   rsi, [rsp+0x60+SIGNED_LONG_LONG_MAX_STR_SIZE]
  mov   rcx, qword [rsp+0x48]
  rep   movsb

  ; add null char
  mov   rax, NULL_CHAR
  stosb

  mov   rax, [rsp+0x50]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, SIGNED_LONG_LONG_MAX_STR_SIZE
  add   rsp, COOKIE_MAX_SIZE
  add   rsp, 0x68
  ret


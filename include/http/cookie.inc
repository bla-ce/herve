section .bss
cookie_t:
  .cookie_name      resq 1
  .cookie_value     resq 1
  .cookie_max_age   resq 1
  .cookie_http_only resq 1
  .cookie_secure    resq 1
  .cookie_domain    resq 1
  .cookie_path      resq 1
  .cookie_samesite  resq 1
cookie_t_end:

section .data
COOKIE_STRUCT_LEN equ cookie_t_end - cookie_t

; cookie struct offsets
COOKIE_OFF_NAME       equ 0x0
COOKIE_OFF_VALUE      equ COOKIE_OFF_NAME + 0x8
COOKIE_OFF_MAX_AGE    equ COOKIE_OFF_VALUE + 0x8
COOKIE_OFF_HTTP_ONLY  equ COOKIE_OFF_MAX_AGE + 0x8
COOKIE_OFF_SECURE     equ COOKIE_OFF_HTTP_ONLY + 0x8
COOKIE_OFF_DOMAIN     equ COOKIE_OFF_SECURE + 0x8
COOKIE_OFF_PATH       equ COOKIE_OFF_DOMAIN + 0x8
COOKIE_OFF_SAMESITE   equ COOKIE_OFF_PATH + 0x8

COOKIE_MAX_SIZE equ 0x1000

ATTRIBUTE_DELIM db "; ", NULL_CHAR

; cookie attributes
MAX_AGE_ATTR    db "Max-Age", NULL_CHAR
SECURE_ATTR     db "Secure", NULL_CHAR
HTTP_ONLY_ATTR  db "HttpOnly", NULL_CHAR
DOMAIN_ATTR     db "Domain", NULL_CHAR
PATH_ATTR       db "Path", NULL_CHAR
SAMESITE_ATTR   db "SameSite", NULL_CHAR

; samesite values
SAMESITE_STRICT db "Strict", NULL_CHAR
SAMESITE_LAX    db "Lax", NULL_CHAR
SAMESITE_NONE   db "None", NULL_CHAR

; token_table: each byte is 0 or 1 indicating invalid or
; valid token char for ASCII code = index in [0..127]
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

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> pointer to the cookie struct
  ; [rsp+0x10]  -> pointer to the serialized cookie struct

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x10], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

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

  ; free serialized cookie (set_header dups it)
  mov   rdi, [rsp+0x10]
  call  free
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rdi, [rsp+0x10]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
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
; @param  rdi: pointer to the name
; @param  rsi: pointer to the value
; @return rax: pointer to the cookie struct
create_cookie:
  sub   rsp, 0x18

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the name
  ; [rsp+0x8]   -> pointer to the value
  ; [rsp+0x10]  -> pointer to the cookie struct

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x10], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; make sure cookie name is valid
  call  is_cookie_key_valid
  cmp   rax, FALSE
  je    .error

  mov   rdi, COOKIE_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  mov   rdi, [rsp]
  mov   [rax+COOKIE_OFF_NAME], rdi

  mov   rdi, [rsp+0x8]
  mov   [rax+COOKIE_OFF_VALUE], rdi

  mov   qword [rax+COOKIE_OFF_MAX_AGE], 0
  mov   qword [rax+COOKIE_OFF_HTTP_ONLY], 0
  mov   qword [rax+COOKIE_OFF_SECURE], 0
  mov   qword [rax+COOKIE_OFF_DOMAIN], 0
  mov   qword [rax+COOKIE_OFF_PATH], 0
  mov   qword [rax+COOKIE_OFF_SAMESITE], 0

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; sets the max age attribute of the cookie
; @param  rdi: pointer to the cookie struct
; @param  rsi: max age of the cookie
; @return rax: return code
set_cookie_max_age:
  cmp   rdi, 0
  jle   .error

  mov   [rdi+COOKIE_OFF_MAX_AGE], rsi

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the domain attribute of the cookie
; @param  rdi: pointer to the cookie struct
; @param  rsi: domain of the cookie
; @return rax: return code
set_cookie_domain:
  cmp   rdi, 0
  jle   .error

  mov   [rdi+COOKIE_OFF_DOMAIN], rsi

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the secure attribute of the cookie
; @param  rdi: pointer to the cookie struct
; @param  rsi: secure attribute
; @return rax: return code
set_cookie_secure:
  cmp   rdi, 0
  jle   .error

  mov   [rdi+COOKIE_OFF_SECURE], rsi

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the path attribute of the cookie
; @param  rdi: pointer to the cookie struct
; @param  rsi: path of the cookie
; @return rax: return code
set_cookie_path:
  cmp   rdi, 0
  jle   .error

  mov   [rdi+COOKIE_OFF_PATH], rsi

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the http only attribute of the cookie
; @param  rdi: pointer to the cookie struct
; @param  rsi: http only attribute
; @return rax: return code
set_cookie_http_only:
  cmp   rdi, 0
  jle   .error

  mov   [rdi+COOKIE_OFF_HTTP_ONLY], rsi

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the same site attribute of the cookie
; @param  rdi: pointer to the cookie struct
; @param  rsi: same site attribute
; @return rax: return code
set_cookie_samesite:
  cmp   rdi, 0
  jle   .error

  mov   [rdi+COOKIE_OFF_SAMESITE], rsi

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the serialization of the cookie
; @param  rdi: pointer to the cookie struct
; @return rax: pointer to the serialized cookie
serialize_cookie:
  sub   rsp, 0x20
  sub   rsp, SIGNED_LONG_LONG_MAX_STR_SIZE

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the cookie struct
  ; [rsp+0x8]   -> pointer to the serialized cookie (boeuf buffer)
  ; [rsp+0x10]  -> pointer to the string
  ; [rsp+0x18]  -> '=' string to append
  ; [rsp+0x20]  -> max age string

  mov   [rsp], rdi
  mov   qword [rsp+0x8], 0
  mov   qword [rsp+0x10], 0

  cmp   rdi, 0
  jle   .error

  ; get cookie name
  mov   rax, [rdi+COOKIE_OFF_NAME]

  ; create boeuf buffer
  mov   rdi, rax
  call  boeuf_create
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; add equal
  mov   byte [rsp+0x18], EQUAL_SIGN
  mov   byte [rsp+0x18+1], NULL_CHAR

  mov   rdi, [rsp+0x8]
  lea   rsi, [rsp+0x18]
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

  ; append domain key
  mov   rdi, [rsp+0x8]
  mov   rsi, DOMAIN_ATTR
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  ; add equal
  mov   rdi, rax
  lea   rsi, [rsp+0x18]
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
  lea   rsi, [rsp+0x18]
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; copy max age string
  mov   rsi, [rsp]
  mov   rdi, qword [rsi+COOKIE_OFF_MAX_AGE]
  lea   rsi, [rsp+0x20]
  mov   rdx, SIGNED_LONG_LONG_MAX_STR_SIZE
  call  itoa
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

  ; add http only
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
  ; path attribute
  mov   rdi, [rsp]
  mov   rax, qword [rdi+COOKIE_OFF_PATH]
  cmp   rax, 0
  je    .skip_path

  ; add '; '
  mov   rdi, [rsp+0x8]
  mov   rsi, ATTRIBUTE_DELIM
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; append path key
  mov   rdi, rax
  mov   rsi, PATH_ATTR
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  ; append equal
  mov   rdi, rax
  lea   rsi, [rsp+0x18]
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; add path
  mov   rdi, [rsp]
  mov   rsi, [rdi+COOKIE_OFF_PATH]
  mov   rdi, [rsp+0x8]
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

.skip_path:
  ; samesite attribute
  mov   rdi, [rsp]
  mov   rax, qword [rdi+COOKIE_OFF_SAMESITE]
  cmp   rax, 0
  je    .skip_samesite

  ; add '; '
  mov   rdi, [rsp+0x8]
  mov   rsi, ATTRIBUTE_DELIM
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; append samesite key
  mov   rdi, rax
  mov   rsi, SAMESITE_ATTR
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  ; append equal
  mov   rdi, rax
  lea   rsi, [rsp+0x18]
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; add samesite
  mov   rdi, [rsp]
  mov   rsi, [rdi+COOKIE_OFF_SAMESITE]
  mov   rdi, [rsp+0x8]
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

.skip_samesite:
  ; duplicate boeuf to return a string
  mov   rdi, [rsp+0x8]
  call  strdup
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  mov   rdi, [rsp+0x8]
  call  boeuf_free
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x10]

  jmp   .return

.error:
  mov   rdi, [rsp+0x10]
  test  rdi, rdi
  jz    .free_boeuf

  call  free

.free_boeuf:
  mov   rdi, [rsp+0x8]
  test  rdi, rdi
  jz    .no_free

  call  boeuf_free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, SIGNED_LONG_LONG_MAX_STR_SIZE
  add   rsp, 0x20
  ret

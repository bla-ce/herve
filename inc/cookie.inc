section .data
  cookie_struct:
    ck_name         times 0x100 db 0 
    ck_value        times 0x100 db 0
    ck_path         times 0x100 db 0
    ck_domain       times 0x100 db 0
    ck_expires      times 0x20  db 0
    ck_max_age      dq 0
    ck_secure       db 0
    ck_http_only    db 0
    ck_same_site    db 0
    ck_partitioned  db 0
  cookie_struct_end:

  CK_STRUCT_LEN equ cookie_struct_end - cookie_struct

  CK_HEADER_KEY db "Set-Cookie", NULL_CHAR

  ; cookie prefixes
  HOST_PREFIX   db "__Host-", NULL_CHAR
  SECURE_PREFIX db "__Secure-", NULL_CHAR

  ; cookie headers
  CK_EXPIRES_HEADER    db "Expires=", NULL_CHAR 
  CK_MAX_AGE_HEADER    db "Max-Age=", NULL_CHAR
  CK_PATH_HEADER       db "Path=", NULL_CHAR
  CK_DOMAIN_HEADER     db "Domain=", NULL_CHAR
  CK_SAME_SITE_HEADER  db "SameSite=", NULL_CHAR

  ; cookie value
  CK_SECURE_VALUE       db "Secure", NULL_CHAR
  CK_HTTP_ONLY_VALUE    db "HttpOnly", NULL_CHAR
  CK_PARTITIONED_VALUE  db "Partitioned", NULL_CHAR

  ; cookie offsets
  CK_OFF_NAME         equ 0x0
  CK_OFF_VALUE        equ CK_OFF_NAME       + 0x100
  CK_OFF_PATH         equ CK_OFF_VALUE      + 0x100
  CK_OFF_DOMAIN       equ CK_OFF_PATH       + 0x100
  CK_OFF_EXPIRES      equ CK_OFF_DOMAIN     + 0x100
  CK_OFF_MAX_AGE      equ CK_OFF_EXPIRES    + 0x20
  CK_OFF_SECURE       equ CK_OFF_MAX_AGE    + 0x8
  CK_OFF_HTTP_ONLY    equ CK_OFF_SECURE     + 0x1
  CK_OFF_SAME_SITE    equ CK_OFF_HTTP_ONLY  + 0x1
  CK_OFF_PARTITIONED  equ CK_OFF_SAME_SITE  + 0x1

  ; same site values
  SAME_SITE_NONE    db "SameSite=None", NULL_CHAR
  SAME_SITE_LAX     db "SameSite=Lax", NULL_CHAR
  SAME_SITE_STRICT  db "SameSite=Strict", NULL_CHAR

  is_token_table  db 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1 ; 33-47
                  db 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0 ; 48-63
                  db 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ; 64-79
                  db 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1 ; 80-95
                  db 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 ; 96-111
                  db 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1 ; 112-126

section .text

; sets cookie header from a cookie struct
; @param  rdi: pointer to cookie struct
; @param  rsi: pointer to headers
; @return rax: return code
set_cookie:
  sub   rsp, 0x18

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; to string
  call  ck_to_string

  cmp   rax, 0
  jl    .error

  ; add to headers
  mov   rdi, [rsp+0x8]
  xor   rsi, rsi
  lea   rsi, [CK_HEADER_KEY]
  mov   rdx, rax
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

; converts cookie struct to string
; @param  rdi: pointer to cookie struct
; @return rax: pointer to string
ck_to_string:
  sub   rsp, 0x10

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

  add   rdi, CK_OFF_NAME
  call  is_cookie_name_valid

  test  rax, rax
  jz    .error

  ; allocate string of HEADER_VALUE_MAX_LEN size
  mov   rdi, HEADER_VALUE_MAX_LEN
  call  malloc

  cmp   rax, 0
  jl    .error

  ; save string onto stack
  mov   [rsp+0x8], rax

  ; serialize cookie struct
    
  
  ; add null char at the end
  mov   rax, NULL_CHAR
  stosb

  mov   rax, [rsp+0x8]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE 

.return:
  add   rsp, 0x10
  ret


; checks if cookie name is valid
; @param  rdi: pointer to cookie name
; @return rax: is valid
is_cookie_name_valid:
  sub   rsp, 0x8

  mov   [rsp], rdi

  call  strlen
  cmp   rax, 0
  jle   .is_not_valid

  mov   r9, rax
  mov   rdi, [rsp]

.loop:
  call  is_token
  test  rax, rax
  jz    .is_not_valid

  inc   rdi
  dec   r9

  test  r9, r9
  jnz   .loop

.is_valid:
  mov   rax, 1
  jmp   .return

.is_not_valid:
  mov   rax, 0

.return:
  add   rsp, 0x8
  ret

; returns whether a given character is a token
; @param  rdi: char
; @return rax: is token
is_token:
  movzx rbx, byte [rdi]

  cmp   rbx, EXCLAMATION_MARK
  jl    .is_not

  cmp   rbx, EQUIVALENCY_SIGN_TILDE
  jg    .is_not

  ; check is token table
  sub   rbx, EXCLAMATION_MARK
  xor   rax, rax
  movzx rax, byte [is_token_table+rbx]

  jmp   .return

.is_not:
  xor   rax, rax

.return:
  ret


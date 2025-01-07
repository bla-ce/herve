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

  COOKIE_STRUCT_LEN equ cookie_struct_end - cookie_struct

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

section .text
  

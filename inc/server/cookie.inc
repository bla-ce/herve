section .bss
cookie_struct:
  name  resq 1
  value resq 1
cookie_struct_end:

section .data
COOKIE_STRUCT_LEN equ cookie_struct_end - cookie_struct

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
; @param  rsi: key of the cookie
; @param  rdx: value of the cookie
; @return rax: return code
set_cookie:
  sub   rsp, 0x30

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  cmp   rdx, 0
  jle   .error

  ; *** STACK USAGE *** ; 
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> pointer to the key
  ; [rsp+0x10]  -> pointer to the value
  ; [rsp+0x18]  -> key length
  ; [rsp+0x20]  -> value length 
  ; [rsp+0x28]  -> pointer to the header value (key=value)

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx

  ; make sure the key is valid
  mov   rdi, [rsp+0x8]
  call  is_cookie_key_valid
  cmp   rax, FALSE
  je    .error

  ; get length of the key and the value
  mov   rdi, [rsp+0x8]
  call  strlen
  cmp   rax, 0
  jl    .error

  mov   qword [rsp+0x18], rax

  mov   rdi, [rsp+0x10]
  call  strlen
  cmp   rax, 0
  jl    .error

  mov   qword [rsp+0x20], rax

  mov   rdi, qword [rsp+0x18]
  add   rdi, qword [rsp+0x20]
  add   rdi, 2    ; add equal sign and null char
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x28], rax

  mov   rdi, rax
  mov   rsi, [rsp+0x8]
  mov   rcx, qword [rsp+0x18]
  rep   movsb

  mov   rax, EQUAL_SIGN
  stosb

  mov   rsi, [rsp+0x10]
  mov   rcx, qword [rsp+0x20]
  rep   movsb

  mov   rax, NULL_CHAR
  stosb

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
  mov   rdx, [rsp+0x28]
  call  set_header
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x30
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


section .data
; headers constants
HEADERS_MAX_COUNT     equ 100
HEADER_KEY_MAX_LEN    equ 0x2000
HEADER_VALUE_MAX_LEN  equ 0x2000
HEADER_LEN            equ HEADER_KEY_MAX_LEN + HEADER_VALUE_MAX_LEN
HEADERS_MAX_LEN       equ HEADER_LEN * HEADERS_MAX_COUNT

; headers key
CONTENT_TYPE_HEADER   db "Content-Type", NULL_CHAR
CONTENT_LENGTH_HEADER db "Content-Length", NULL_CHAR
CONNECTION_HEADER     db "Connection", NULL_CHAR

; content type
CONTENT_HTML  db "text/html; charset=UTF-8", NULL_CHAR
CONTENT_CSS   db "text/css; charset=UTF-8", NULL_CHAR
CONTENT_JS    db "text/javascript; charset=UTF-8", NULL_CHAR
CONTENT_PLAIN db "text/plain; charset=UTF-8", NULL_CHAR
CONTENT_BIN   db "application/octet-stream", NULL_CHAR
CONTENT_CSV   db "text/csv; charset=UTF-8", NULL_CHAR
CONTENT_GIF   db "image/gif", NULL_CHAR
CONTENT_JPEG  db "image/jpeg", NULL_CHAR
CONTENT_JSON  db "application/json; charset=UTF-8", NULL_CHAR
CONTENT_MP4   db "video/mp4", NULL_CHAR
CONTENT_PNG   db "image/png", NULL_CHAR
CONTENT_SVG   db "image/svg+xml", NULL_CHAR
CONTENT_XML   db "application/xml", NULL_CHAR

; connection
CONNECTION_CLOSE db "close", NULL_CHAR

section .text
; load the headers into the response
; @param  rdi: pointer to headers array
; @return rax: pointer to string
stringify_headers:
  sub   rsp, 0x18

  cmp   rdi, 0
  jle   .error

  mov   [rsp], rdi
  mov   qword [rsp+0x8], 0 ; headers index

  ; malloc string
  mov   rdi, HEADERS_MAX_LEN
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax
  mov   rdi, rax

.loop:
  mov   rsi, [rsp]
  mov   rax, qword [rsp+0x8]
  mov   rbx, HEADER_LEN
  mul   rbx
  add   rsi, rax

  cmp   byte [rsi], NULL_CHAR
  je    .skip_cpy

  ; load header
  call  strcpy

  ; load new line
  xor   rax, rax
  mov   al, CARRIAGE_RETURN
  stosb
  mov   al, LINE_FEED
  stosb

.skip_cpy:
  inc   qword [rsp+0x8]
  mov   rax, HEADERS_MAX_COUNT
  cmp   qword [rsp+0x8], rax
  jge   .load_post_headers

  jmp   .loop

.load_post_headers:
  ; load post_headers
  xor   rax, rax
  mov   al, CARRIAGE_RETURN
  stosb
  mov   al, LINE_FEED
  stosb

  mov   rax, [rsp+0x10]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; set the headers
; @param  rdi: pointer to headers has table
; @param  rsi: key
; @param  rdx: value
; @return rax: return code
set_header:
  sub   rsp, 0x18

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  cmp   rdx, 0
  jle   .error

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx

  mov   rdi, [rsp+0x8]

  ; get canonical format of header key
  call  canonical_mime_header_key
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, rax
  mov   rdx, [rsp+0x10]
  call  ht_insert

  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; gets value of header, returns -1 if does not exist
; @param  rdi: headers
; @param  rsi: key
; @return rax: pointer to value
get_header_value:
  sub   rsp, 0x10

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  ; get canonical format of the header key
  mov   rdi, rsi
  call  canonical_mime_header_key
  cmp   rax, 0
  jl    .error

  ; save canonical format
  mov   rdi, rax
  mov   [rsp+0x8], rdi

  ; make sure the header key exist
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  call  ht_get

  cmp   rax, 0
  jl    .error

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; returns the canonical format of the MIME header key
; @param  rdi: header key
; @return rax: canonical format of the header key
canonical_mime_header_key:
  sub   rsp, 0x10

  cmp   rdi, 0
  jle   .error

  mov   [rsp], rdi
  mov   qword [rsp+0x8], 1 ; next char needs to be upper

  mov   rsi, [rsp]

.loop:
  xor   rax, rax
  mov   al, byte [rsi]

  ; check if it's valid char
  cmp   al, 0
  jl    .error
  je    .return ; end of string

  cmp   al, LOWERCASE_Z
  jg    .error

.check_hyphen:
  cmp   al, HYPHEN
  je    .is_hyphen

.check_upper:
  cmp   al, UPPERCASE_A
  jl    .error

  cmp   al, UPPERCASE_Z
  jl    .is_upper

.check_lower:
  cmp   al, LOWERCASE_A
  jl    .error

  cmp   al, LOWERCASE_Z
  jle   .is_lower

  jmp   .error

.is_hyphen:
  mov   qword [rsp+0x8], 1
  inc   rsi
  jmp   .loop

.is_lower:
  cmp   qword [rsp+0x8], 0
  jne   .upper

  inc   rsi
  jmp    .loop
  
.upper:
  sub   byte [rsi], LOWER_TO_UPPER

  mov   qword [rsp+0x8], 0
  inc   rsi
  jmp   .loop

.is_upper:
  mov   qword [rsp+0x8], 0
  inc   rsi
  jmp   .loop

.error:
  mov   rax, FAILURE_CODE
  add   rsp, 0x10
  ret

.return:
  mov   rax, [rsp]
  add   rsp, 0x10
  ret


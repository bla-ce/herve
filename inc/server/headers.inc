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
HOST_HEADER           db "Host", NULL_CHAR
LOCATION_HEADER       db "Location", NULL_CHAR
AUTHORIZATION_HEADER  db "Authorization", NULL_CHAR

; content type
CONTENT_HTML  db "text/html; charset=UTF-8", NULL_CHAR
CONTENT_APL   db "text/html; charset=UTF-8", NULL_CHAR
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

CRLF  db CARRIAGE_RETURN, LINE_FEED, CARRIAGE_RETURN, LINE_FEED, NULL_CHAR

section .text
; parses the headers string into a hash table
; @param  rdi: pointer to headers string
; @return rax: pointer to the hash table
parse_headers:
  sub   rsp, 0x20

  cmp   rdi, 0
  jle   .error ; TODO: will have to confirm if it can be null

  mov   [rsp], rdi

  ; get the number of headers (count ':')
  mov   rdi, [rsp]
  mov   rsi, LINE_FEED
  call  count_char

  cmp   rax, 0
  jl    .error

  ; add one for the last header
  inc   rax

  ; create a hash table for n entries
  mov   rdi, rax
  call  ht_create

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; load headers string
  mov   rdi, [rsp]
  mov   [rsp+0x10], rdi

.loop:
; look for the following colon  
  mov   rdi, [rsp+0x10]   
  mov   rsi, COLON
  call  find_next_char

  cmp   rax, 0
  jl    .error
  je    .end_loop ; TODO: is it an error?

  ; go to after the colon
  mov   rdi, [rsp+0x10]
  mov   rsi, rdi
  add   rsi, rax
  mov   [rsp+0x10], rsi

  ; strndup the key
  mov   rsi, rax
  call  strndup

  cmp   rax, 0
  jl    .error

  ; save the key
  mov   [rsp+0x18], rax

  ; get the following carriage return
  mov   rsi, [rsp+0x10]
  
  ; next characters should be ':' and ' '
  xor   rax, rax
  lodsb

  mov   rdi, rsi

  cmp   rax, COLON
  jne   .error

  lodsb ; check if there is a space after
  cmp     rax, SPACE
  cmove   rdi, rsi    ; if the next char is not a space, go back one   

  mov   [rsp+0x10], rdi

  mov   rsi, CARRIAGE_RETURN 
  call  find_next_char

  cmp   rax, 0
  jl    .last_key

  jmp   .copy_value

.last_key:
  mov   rdi, [rsp+0x10]
  call  strlen

  cmp   rax, 0
  jl    .error

.copy_value:
  ; strndup the value
  mov   rdi, [rsp+0x10]
  mov   rsi, rdi
  add   rsi, rax
  mov   [rsp+0x10], rsi
  mov   rsi, rax
  call  strndup

  cmp   rax, 0
  jl    .error

  ; insert into the hash table
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x18]
  mov   rdx, rax
  call  ht_insert

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x10]
  cmp   byte [rdi], NULL_CHAR
  je    .end_loop

  ; add two to go after CRLF
  add   rdi, 2
  mov   [rsp+0x10], rdi

  jmp   .loop

.end_loop:
  mov   rax, [rsp+0x8]

  jmp   .return

.empty:
  xor   rax, rax
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret

; load the headers into the response
; @param  rdi: pointer to headers hash table
; @return rax: pointer to string
stringify_headers:
  sub   rsp, 0x38

  cmp   rdi, 0
  jle   .error

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to headers hash table
  ; [rsp+0x8]   -> headers count
  ; [rsp+0x10]  -> array of keys
  ; [rsp+0x18]  -> key address
  ; [rsp+0x20]  -> headers string dynamic size
  ; [rsp+0x28]  -> pointer to headers string
  ; [rsp+0x30]  -> dynamic pointer headers string

  mov   [rsp], rdi
  mov   qword [rsp+0x20], 0
  mov   qword [rsp+0x28], 0

  ; get hash table length
  mov   rdi, [rsp]
  call  ht_get_length

  cmp   rax, 0
  jl    .error

  mov   rbx, HEADERS_MAX_COUNT
  
  ; make sure length does not exceed HEADERS_MAX_COUNT
  cmp   rax, rbx
  cmovg rax, rbx

  mov   qword [rsp+0x8], rax

  ; get all keys
  mov   rdi, [rsp]
  call  ht_get_keys

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  xor   r9, r9

  ; get total length of the headers
.loop:
  cmp   r9, qword [rsp+0x8]
  jge   .end_loop

  ; get array of keys
  mov   rsi, [rsp+0x10]

  ; load key
  mov   rdi, [rsi+r9*8]

  ; save key address
  mov   [rsp+0x18], rdi

  ; get key length
  mov   rdi, [rsp+0x18]
  call  strlen

  cmp   rax, 0
  jl    .empty_header

  add   qword [rsp+0x20], rax
  add   qword [rsp+0x20], 2     ; colon and space

  ; get value
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x18]
  call  ht_get

  cmp   rax, 0
  jl    .error

  ; get value length
  mov   rdi, rax
  call  strlen

  cmp   rax, 0
  jl    .error

  add   qword [rsp+0x20], rax
  add   qword [rsp+0x20], 2     ; CRLF

.empty_header:
  inc   r9

  jmp   .loop
.end_loop:

  mov   rdi, qword [rsp+0x20]
  add   rdi, 3      ; CRLF and NULL_CHAR
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x28], rax
  mov   [rsp+0x30], rax

  xor   r9, r9

  ; for each key
.loop2:
  cmp   r9, qword [rsp+0x8]
  jge   .end_loop2

  ; get array of keys
  mov   rsi, [rsp+0x10]

  ; load key
  mov   rdi, [rsi+r9*8]

  ; save key address
  mov   [rsp+0x18], rdi

  ; copy key
  mov   rdi, [rsp+0x30]
  mov   rsi, [rsp+0x18]
  call  strcpy

  cmp   rax, 0
  jl    .empty_header2

  ; load ':'
  mov   rdi, rax
  mov   rax, COLON
  stosb

  ; load ' '
  mov   rax, SPACE
  stosb

  mov   [rsp+0x30], rdi

  ; get value
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x18]
  call  ht_get

  cmp   rax, 0
  jl    .error

  ; load value 
  mov   rdi, [rsp+0x30]
  mov   rsi, rax
  call  strcpy

  cmp   rax, 0
  jl    .error

  ; load new line
  mov   rdi, rax
  mov   rax, CARRIAGE_RETURN
  stosb

  mov   rax, LINE_FEED
  stosb

.empty_header2:
  inc   r9

  mov   [rsp+0x30], rdi

  jmp   .loop2
.end_loop2:

  ; load post headers
  mov   rdi, [rsp+0x30]
  mov   rax, CARRIAGE_RETURN
  stosb

  mov   rax, LINE_FEED
  stosb

  mov   rax, NULL_CHAR
  stosb
  
  mov   rax, [rsp+0x28]
  
  jmp   .return

.error:
  ; free headersif mallocd
  mov   rdi, [rsp+0x28]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x38
  ret

; set the headers
; @param  rdi: pointer to headers hash table
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

  ; make sure key and value length are below max lengths
  mov   rdi, [rsp+0x8]
  call  strlen

  cmp   rax, 0
  jl    .error

  cmp   rax, HEADER_KEY_MAX_LEN
  jg    .error

  mov   rdi, [rsp+0x10]
  call  strlen

  cmp   rax, 0
  jl    .error

  cmp   rax, HEADER_VALUE_MAX_LEN
  jg    .error

  ; get canonical format of header key
  mov   rdi, [rsp+0x8]
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
  mov   qword [rsp+0x8], TRUE
  inc   rsi
  jmp   .loop

.is_lower:
  cmp   qword [rsp+0x8], FALSE
  jne   .upper

  inc   rsi
  jmp    .loop
  
.upper:
  sub   byte [rsi], LOWER_TO_UPPER

  mov   qword [rsp+0x8], FALSE
  inc   rsi
  jmp   .loop

.is_upper:
  mov   qword [rsp+0x8], FALSE
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


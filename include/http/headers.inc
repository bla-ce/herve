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
COOKIE_HEADER         db "Cookie", NULL_CHAR
SET_COOKIE_HEADER     db "Set-Cookie", NULL_CHAR

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
CONNECTION_CLOSE      db "close", NULL_CHAR
CONNECTION_KEEP_ALIVE db "keep-alive", NULL_CHAR

FORM_ENCODED db "application/x-www-form-urlencoded", NULL_CHAR

CRLF  db CARRIAGE_RETURN, LINE_FEED, CARRIAGE_RETURN, LINE_FEED, NULL_CHAR

section .text
; parses the headers string into a hash table
; @param  rdi: pointer to headers string
; @return rax: pointer to the hash table
parse_headers:
  sub   rsp, 0x28

  ; STACK USAGE
  ; [rsp]       -> pointer to the header strings
  ; [rsp+0x8]   -> pointer to the hash table structure
  ; [rsp+0x10]  -> dynamic pointer traversing the string
  ; [rsp+0x18]  -> pointer to the dup key
  ; [rsp+0x20]  -> pointer to the dup value

  mov   [rsp], rdi
  mov   qword [rsp+0x8], 0
  mov   qword [rsp+0x18], 0
  mov   qword [rsp+0x20], 0

  cmp   rdi, 0
  jle   .error ; TODO: will have to confirm if it can be null

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
  je    .loop_end ; TODO: is it an error?

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
  ; get pointer to the value
  mov   rdi, [rsp+0x10]
  mov   rsi, rdi

  ; move pointer after the value
  add   rsi, rax
  mov   [rsp+0x10], rsi

  ; duplicate the value
  mov   rsi, rax
  call  strndup
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  ; insert into the hash table
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x18]
  mov   rdx, [rsp+0x20]
  call  ht_insert
  cmp   rax, 0
  jl    .error

  ; free key
  mov   rdi, [rsp+0x18]
  call  free
  cmp   rax, 0
  jl    .error

  ; free value
  mov   rdi, [rsp+0x20]
  call  free
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x10]
  cmp   byte [rdi], NULL_CHAR
  je    .loop_end

  ; add two to go after CRLF
  add   rdi, 2
  mov   [rsp+0x10], rdi

  jmp   .loop

.loop_end:
  mov   rax, [rsp+0x8]

  jmp   .return

.empty:
  xor   rax, rax
  jmp   .return

.error:
  mov   rdi, [rsp+0x18]
  test  rdi, rdi
  jz    .free_value

  call  free

.free_value:
  mov   rdi, [rsp+0x20]
  test  rdi, rdi
  jz    .free_ht

  call  free

.free_ht:
  mov   rdi, [rsp+0x8]
  test  rdi, rdi
  jz    .no_free

  call  ht_free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x28
  ret

; load the headers into the response
; @param  rdi: pointer to headers hash table
; @return rax: pointer to string
stringify_headers:
  sub   rsp, 0x38

  ; STACK USAGE
  ; [rsp]       -> pointer to headers hash table
  ; [rsp+0x8]   -> headers count
  ; [rsp+0x10]  -> array of keys
  ; [rsp+0x18]  -> key address
  ; [rsp+0x20]  -> headers string dynamic size
  ; [rsp+0x28]  -> pointer to headers string
  ; [rsp+0x30]  -> dynamic pointer headers string

  mov   [rsp], rdi
  mov   qword [rsp+0x10], 0
  mov   qword [rsp+0x20], 0
  mov   [rsp+0x28], rsi

  cmp   rdi, 0
  jle   .error

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
  jge   .loop_end

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
  jle   .error

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
.loop_end:

  mov   rax, [rsp+0x28]
  mov   [rsp+0x30], rax

  xor   r9, r9

  ; for each key
.loop2:
  cmp   r9, qword [rsp+0x8]
  jge   .loop_end2

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
  jle   .error

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
.loop_end2:

  ; load post headers
  mov   rdi, [rsp+0x30]
  mov   rax, CARRIAGE_RETURN
  stosb

  mov   rax, LINE_FEED
  stosb

  mov   rax, NULL_CHAR
  stosb

  ; free list of keys
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
  jle   .error

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

; extracts extension of a file and returns content type header
; @param  rdi: pointer to file path
; @return rax: pointer to content type header
get_content_type:
  sub   rsp, 0x8

  cmp   rdi, 0
  jle   .not_supported

  ; find next dot
  mov   rsi, DOT
  call  find_next_char

  cmp   rax, 0
  jl    .no_ext

  ; go to dot
  add   rdi, rax

  ; save '.' position
  mov   [rsp], rdi

  lea   rdi, [HTML_EXT]
  mov   rsi, [rsp]
  call  strcmp

  cmp   rax, 1
  jne   .check_apl

  lea   rax, [CONTENT_HTML]
  jmp   .return

.check_apl:
  lea   rdi, [APL_EXT]
  mov   rsi, [rsp]
  call  strcmp

  cmp   rax, 1
  jne   .check_js

  lea   rax, [CONTENT_APL] ; template extension
  jmp   .return

.check_js:
  lea   rdi, [JS_EXT]
  mov   rsi, [rsp]
  call  strcmp

  cmp   rax, 1
  jne   .check_css

  lea   rax, [CONTENT_JS]
  jmp   .return

.check_css:
  lea   rdi, [CSS_EXT]
  mov   rsi, [rsp]
  call  strcmp

  cmp   rax, 1
  jne   .check_bin

  lea   rax, [CONTENT_CSS]
  jmp   .return

.check_bin:
  lea   rdi, [BIN_EXT]
  mov   rsi, [rsp]
  call  strcmp

  cmp   rax, 1
  jne   .check_csv

  lea   rax, [CONTENT_BIN]
  jmp   .return

.check_csv:
  lea   rdi, [CSV_EXT]
  mov   rsi, [rsp]
  call  strcmp

  cmp   rax, 1
  jne   .check_gif

  lea   rax, [CONTENT_CSV]
  jmp   .return

.check_gif:
  lea   rdi, [GIF_EXT]
  mov   rsi, [rsp]
  call  strcmp

  cmp   rax, 1
  jne   .check_jpeg

  lea   rax, [CONTENT_GIF]
  jmp   .return

.check_jpeg:
  lea   rdi, [JPEG_EXT]
  mov   rsi, [rsp]
  call  strcmp

  cmp   rax, 1
  jne   .check_jpg

  lea   rax, [CONTENT_JPEG]
  jmp   .return

.check_jpg:
  lea   rdi, [JPG_EXT]
  mov   rsi, [rsp]
  call  strcmp

  cmp   rax, 1
  jne   .check_json

  lea   rax, [CONTENT_JPEG]
  jmp   .return

.check_json:
  lea   rdi, [JSON_EXT]
  mov   rsi, [rsp]
  call  strcmp

  cmp   rax, 1
  jne   .check_mp4

  lea   rax, [CONTENT_JSON]
  jmp   .return

.check_mp4:
  lea   rdi, [MP4_EXT]
  mov   rsi, [rsp]
  call  strcmp

  cmp   rax, 1
  jne   .check_png

  lea   rax, [CONTENT_MP4]
  jmp   .return

.check_png:
  lea   rdi, [PNG_EXT]
  mov   rsi, [rsp]
  call  strcmp

  cmp   rax, 1
  jne   .check_svg

  lea   rax, [CONTENT_PNG]
  jmp   .return

.check_svg:
  lea   rdi, [SVG_EXT]
  mov   rsi, [rsp]
  call  strcmp

  cmp   rax, 1
  jne   .check_xml

  lea   rax, [CONTENT_SVG]
  jmp   .return

.check_xml:
  lea   rdi, [XML_EXT]
  mov   rsi, [rsp]
  call  strcmp

  cmp   rax, 1
  jne   .check_plain

  lea   rax, [CONTENT_XML]
  jmp   .return

.check_plain:
  lea   rdi, [TEXT_EXT]
  mov   rsi, [rsp]
  call  strcmp

  cmp   rax, 1
  jne   .not_supported

  lea   rax, [CONTENT_PLAIN]
  jmp   .return

.no_ext:
  mov   rax, FAILURE_CODE
  jmp   .return

.not_supported:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

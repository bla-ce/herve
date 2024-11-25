section .bss
response_headers  resb HEADERS_MAX_LEN
buf_header_value  resb HEADER_VALUE_MAX_LEN

section .data
; headers constants
HEADERS_MAX_COUNT     equ 0x20
HEADER_KEY_MAX_LEN    equ 0x2000
HEADER_VALUE_MAX_LEN  equ 0x2000
HEADER_LEN            equ HEADER_KEY_MAX_LEN + HEADER_VALUE_MAX_LEN
HEADERS_MAX_LEN       equ HEADER_LEN * HEADERS_MAX_COUNT

CONTENT_TYPE_HEADER   db "Content-Type", NULL_CHAR
CONTENT_LENGTH_HEADER db "Content-Length", NULL_CHAR
CONNECTION_HEADER     db "Connection", NULL_CHAR

section .text
; load the headers into the response
; @param  rdi: response
; @param  rsi: headers
; @return rax: points to the response after headers
load_headers:
  sub   rsp, 0x10

  mov   qword [rsp], 0 ; header count
  mov   [rsp+0x8], rsi

.loop:
  mov   rsi, [rsp+0x8]
  mov   rax, qword [rsp]
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
  inc   qword [rsp]
  mov   rax, HEADERS_MAX_COUNT
  cmp   qword [rsp], rax
  jge   .return

  jmp   .loop
  
.return:
  ; load post_headers
  xor   rax, rax
  mov   al, CARRIAGE_RETURN
  stosb
  mov   al, LINE_FEED
  stosb

  lea   rax, [rdi]

  add   rsp, 0x10
  ret

; set the headers
; @param  rdi: headers
; @param  rsi: key
; @param  rdx: value
; @return rax: return code
set_header:
  sub   rsp, 0x20

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx

  ; get key length
  mov   rdi, [rsp+0x8]
  call  strlen
  cmp   rax, HEADER_KEY_MAX_LEN
  jge   .error

  ; get value length
  mov   rdi, [rsp+0x10]
  call  strlen
  cmp   rax, HEADER_VALUE_MAX_LEN
  jge   .error

  mov   rdi, [rsp+0x8]

  ; get canonical format of header key
  call  canonical_mime_header_key
  cmp   rax, 0
  jl    .error

  lea   rdi, [rax]

  ; save header key
  mov   [rsp+0x8], rdi

  ; check if the key is already in the headers
  lea   rsi, [rdi]
  mov   rdi, [rsp]
  call  header_exist
  cmp   rax, 0
  jge   .exist

  mov   qword [rsp+0x18], 0 ; header count
  
  mov   rdi, [rsp]

.loop:
  ; check if index is empty
  cmp   byte [rdi], NULL_CHAR
  je    .set

  ; go to next index
  inc   qword [rsp+0x18]
  add   rdi, HEADER_LEN

  ; make sure we didn't exceed HEADERS_MAX_COUNT 
  mov   rax, HEADERS_MAX_COUNT
  cmp   qword [rsp+0x18], rax
  jg    .error

  jmp   .loop

.set:
  ; copy key
  mov   rsi, [rsp+0x8]
  call  strcpy

  ; copy colon
  xor   rax, rax
  mov   al, COLON
  stosb

  ; copy space
  xor   rax, rax
  mov   al, SPACE
  stosb

  ; copy value
  mov   rsi, [rsp+0x10]
  call  strcpy
  
  mov   rax, SUCCESS_CODE
  jmp   .return

.exist:
  ; if header exists, overwrite
  mov   rdi, [rsp]
  
  ; rax is already populated
  xor   rdx, rdx
  mov   rbx, HEADER_LEN
  mul   rbx

  add   rdi, rax

  ; empty index
  xor   rax, rax
  mov   rcx, HEADER_LEN
  rep   stosb

  sub   rdi, HEADER_LEN
  jmp   .set

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret

; check if header key already in headers
; @param  rdi: headers
; @param  rsi: key
; @return rax: index of the key
header_exist:
  sub   rsp, 0x20

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x10], 0 ; header_count

.loop:
  ; point to next header
  mov   rdi, [rsp]
  xor   rdx, rdx
  mov   rax, qword [rsp+0x10]
  mov   rbx, HEADER_LEN
  mul   rbx
  add   rdi, rax

  ; save rdi position
  mov   [rsp+0x18], rdi

  ; get current key length
  mov   rdi, [rsp+0x18]
  mov   rsi, [rsp+0x8]
  call  strpos

  cmp  rax, 0
  je   .equal

  inc   qword [rsp+0x10]

  ; make sure we didn't exceed HEADERS_MAX_COUNT
  mov   rax, HEADERS_MAX_COUNT
  cmp   qword [rsp+0x10], rax
  jg    .not_equal

  jmp   .loop

.not_equal:
  mov   rax, FAILURE_CODE
  jmp   .return

.equal:
  ; returns the index of the header
  mov   rax, qword [rsp+0x10]

.return:
  add   rsp, 0x20
  ret


; gets value of header, returns -1 if does not exist
; @param  rdi: headers
; @param  rsi: key
; @return rax: pointer to value
get_header_value:
  sub   rsp, 0x10

  mov   [rsp], rdi

  ; get canonical format of the header key
  lea   rdi, [rsi]
  call  canonical_mime_header_key
  cmp   rax, 0
  jl    .error

  ; save canonical format
  lea   rdi, [rax]
  mov   [rsp+0x8], rdi

  ; make sure the header key exist
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  call  header_exist
  cmp   rax, 0
  jl    .return

  ; rax contains the index
  mov   rdi, [rsp]
  mov   rbx, HEADER_LEN
  mul   rbx
  add   rdi, rax

  mov   rsi, COLON
  call  find_next_char

  ; go to ':' and add two to go after the space
  add   rdi, rax
  add   rdi, 2

  ; save rdi onto the stack
  mov   [rsp+0x8], rdi

  ; clear value buffer
  xor   rax, rax
  lea   rdi, [buf_header_value]
  mov   rcx, HEADER_VALUE_MAX_LEN
  rep   stosb

  ; get header value
  lea   rdi, [buf_header_value]
  mov   rsi, [rsp+0x8]
  call  strcpy

  lea   rax, [buf_header_value]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret


; delete header, returns -1 if does not exist
; @param  rsi: headers
; @param  rdi: key
; @return rax: code
del_header:
  sub   rsp, 0x10

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  ; get index of the header
  call  header_exist

  cmp   rax, 0
  jl    .error

  xor   rdx, rdx
  mov   rbx, HEADER_LEN
  mul   rbx

  mov   rdi, [rsp]
  add   rdi, rax

  ; clear header
  mov   rcx, HEADER_LEN
  rep   stosb
  
  mov   rax, SUCCESS_CODE
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

  cmp   al, LOW_Z_CHAR
  jg    .error

.check_hyphen:
  cmp   al, HYPHEN
  je    .is_hyphen

.check_upper:
  cmp   al, UPPER_A_CHAR
  jl    .error

  cmp   al, UPPER_Z_CHAR
  jl    .is_upper

.check_lower:
  cmp   al, LOW_A_CHAR
  jl    .error

  cmp   al, LOW_Z_CHAR
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

; get response headers
; @return rax: pointer response headers
get_headers:
  lea   rax, [response_headers]
  ret


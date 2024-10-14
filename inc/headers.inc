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
; @param  rdi: response pointer
; @return rax: response pointer after headers
load_headers:
  sub   rsp, 0x10

  mov   [rsp], rdi
  lea   rsi, [rdi]
  mov   qword [rsp+0x8], 0 ; header count

.loop:
  lea   rsi, [response_headers]
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
; @param  rdi: key
; @param  rsi: value
; @return rax: return code
set_header:
  sub   rsp, 0x18

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  call  strlen
  cmp   rax, HEADER_KEY_MAX_LEN
  jge   .error

  lea   rax, [rsp+0x8]
  mov   rdi, [rax]
  call  strlen
  cmp   rax, HEADER_VALUE_MAX_LEN
  jge   .error

  lea   rax, [rsp]
  mov   rdi, [rax]

  call  header_exist
  cmp   rax, 0
  jge   .exist

  mov   qword [rsp+0x10], 0 ; header count
  
  lea   rdi, [response_headers]

.loop:
  cmp   byte [rdi], NULL_CHAR
  je    .set

  inc   qword [rsp+0x10]
  add   rdi, HEADER_LEN

  mov   rax, HEADERS_MAX_COUNT
  cmp   qword [rsp+0x10], rax
  jg    .error

  jmp   .loop

.set:
  ; copy key
  lea   rax, [rsp]
  mov   rsi, [rax]
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
  lea   rax, [rsp+0x8]   
  mov   rsi, [rax]
  call  strcpy
  
  mov   rax, SUCCESS_CODE
  jmp   .return

.exist:
  lea   rdi, [response_headers]
  
  ; rax is already populated
  xor   rdx, rdx
  mov   rbx, HEADER_LEN
  mul   rbx

  add   rdi, rax

  xor   rax, rax
  mov   rcx, HEADER_LEN
  rep   stosb

  sub   rdi, HEADER_LEN
  jmp   .set

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; check if header key already in headers
; @param  rdi: key
; @return rax: index of the key
header_exist:
  sub   rsp, 0x18

  mov   [rsp], rdi
  mov   qword [rsp+0x8], 0 ; header_count

.loop:
  ; point to next header
  lea   rdi, [response_headers]
  xor   rdx, rdx
  mov   rax, qword [rsp+0x8]
  mov   rbx, HEADER_LEN
  mul   rbx
  add   rdi, rax

  ; save rdi position
  mov   [rsp+0x10], rdi

  lea   rax, [rsp]
  mov   rdi, [rax]
  call  strlen
  mov   r9, rax
  lea   rdx, [rdi]

  lea   rax, [rsp+0x10]
  mov   rdi, [rax]
  call  strlen
  mov   rsi, rax
  mov   rcx, r9
  call  strpos

  cmp  rax, 0
  jge  .equal

  inc   qword [rsp+0x8]

  mov   rax, HEADERS_MAX_COUNT
  cmp   qword [rsp+0x8], rax
  jg    .not_equal

  jmp   .loop

.not_equal:
  mov   rax, -1
  jmp   .return

.equal:
  mov   rax, qword [rsp+0x8]

.return:
  add   rsp, 0x18
  ret


; gets value of header, returns -1 if does not exist
; @param  rdi: key
; @return rax: pointer to value
get_header:
  sub   rsp, 0x8
  
  mov   [rsp], rdi

  call  header_exist
  cmp   rax, 0
  jl    .return

  ; rax contains the index
  lea   rdi, [response_headers]
  mov   rbx, HEADER_LEN
  mul   rbx
  add   rdi, rax

  mov   rsi, COLON
  call  find_next_char

  ; go to ':' and add two to go after the space
  add   rdi, rax
  add   rdi, 2

.b1:
  ; save rdi onto the stack
  mov   [rsp], rdi

  ; clear value buffer
  xor   rax, rax
  lea   rdi, [buf_header_value]
  mov   rcx, HEADER_VALUE_MAX_LEN
  rep   stosb

  lea   rdi, [buf_header_value]
  lea   rax, [rsp]
  mov   rsi, [rax]
  call  strcpy

  lea   rax, [buf_header_value]

.return:
  add   rsp, 0x8
  ret



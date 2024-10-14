.section data
; headers constants
HEADERS_MAX_COUNT     equ 0x20
HEADER_KEY_MAX_LEN    equ 0x20
HEADER_VALUE_MAX_LEN  equ 0x20
HEADER_LEN            equ HEADER_KEY_MAX_LEN + HEADER_VALUE_MAX_LEN
HEADERS_MAX_LEN       equ HEADER_LEN * HEADERS_MAX_COUNT

.section text
; set the headers
; @param  rdi: key
; @param  rsi: value
; @return rax: return code
set_headers:
  sub   rsp, 0x18

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  call  header_exist
  test  rax, rax
  jnz   .error

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

  ; copy value
  lea   rax, [rsp+0x8]   
  mov   rsi, [rax]
  call  strcpy
  
  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret


; check if header key already in headers
; @param  rdi: key
; @return rax: (bool)exist
header_exist:
  sub   rsp, 0x10

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

  lea   rax, [rsp]
  mov   rsi, [rax]
  call  cmpstr

  test  rax, rax
  jnz   .equal

  inc   qword [rsp+0x8]

  mov   rax, HEADERS_MAX_COUNT
  cmp   qword [rsp+0x8], rax
  jg    .not_equal

  jmp   .loop

.not_equal:
  mov   rax, 0
  jmp   .return

.equal:
  mov   rax, 1

.return:
  add   rsp, 0x10
  ret

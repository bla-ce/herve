section .data

base64_std_alphabet db "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    db "abcdefghijklmnopqrstuvwxyz"
                    db "0123456789+/"

BASE64_MAX_INDEX equ 63

section .text

; encodes n amount of bytes using base64
; user is responsible for freeing the encoded string
; @param  rdi: bytes to encode
; @param  rsi: number of bytes to encode
; @return rax: pointer to the encoded string
base64_encode:
  sub   rsp, 0x28

  ; STACK USAGE
  ; [rsp]       -> pointer to the string to encode
  ; [rsp+0x8]   -> number of bytes to encode
  ; [rsp+0x10]  -> size of the encoded string
  ; [rsp+0x18]  -> pointer to the encoded string
  ; [rsp+0x20]  -> dynamic pointer

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x18], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; get size of encoded string
  mov   rdi, [rsp+0x8]
  call  _base64_get_encoded_size
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; malloc string
  mov   rdi, rax
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax
  mov   [rsp+0x20], rax

.loop:
  cmp   qword [rsp+0x8], 0
  je    .end

  cmp   qword [rsp+0x8], 3
  jl    .loop_end

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x20]
  call  _base64_encode_chunk
  cmp   rax, 0
  jl    .error

  ; dec number of bytes to encode
  sub   qword [rsp+0x8], 3

  ; go past the encoded part
  mov   rdi, [rsp+0x20]
  add   rdi, 4
  mov   [rsp+0x20], rdi

  mov   rdi, [rsp]
  add   rdi, 3
  mov   [rsp], rdi

  jmp   .loop

.loop_end:
  ; handling padding here
  ; get the first 6-bit of the first byte
  xor   rax, rax
  mov   rdi, [rsp]
  mov   al, byte [rdi]
  shr   rax, 0x2

  ; get the character from that value
  mov   rdi, rax
  call  _base64_get_char
  cmp   rax, 0
  jl    .error

  ; add character to the dest
  mov   rdi, [rsp+0x20]
  stosb

  mov   [rsp+0x20], rdi

  ; get the last 2-bit of the first byte
  xor   rax, rax
  mov   rdi, [rsp]
  mov   al, byte [rdi]
  and   rax, 0x3
  ; these bits are the high bit of the 2nd chunk
  shl   al, 0x4

  ; get the first 4-bit of the second byte
  xor   rbx, rbx
  mov   bl, byte [rdi+1]
  shr   bl, 0x4

  or    al, bl
  mov   rdi, rax
  call  _base64_get_char
  cmp   rax, 0
  jl    .error

  ; add character to the dest
  mov   rdi, [rsp+0x20]
  stosb

  mov   [rsp+0x20], rdi

  cmp   qword [rsp+0x8], 1
  je    .padding_1

  ; encode last byte
  ; get the last 4-bit of the second byte
  xor   rax, rax
  mov   rdi, [rsp]
  mov   al, byte [rdi+1]
  and   al, 0xF
  ; these bits are the high bit of the 3rd chunk
  shl   al, 0x2

  mov   rdi, rax
  call  _base64_get_char
  cmp   rax, 0
  jl    .error

  ; add character to the dest
  mov   rdi, [rsp+0x20]
  stosb

  ; add pad char
  mov   rax, EQUAL_SIGN
  stosb

  mov   [rsp+0x20], rdi
  jmp   .end

.padding_1:
  mov   rdi, [rsp+0x20]
  mov   rax, EQUAL_SIGN
  mov   rcx, 2
  rep   stosb

  mov   [rsp+0x20], rdi

.end:
  ; add null char at the end
  mov   rdi, [rsp+0x20]
  mov   byte [rdi], NULL_CHAR

  mov   rax, [rsp+0x18]
  jmp   .return

.error:
  mov   rdi, [rsp+0x18]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x28
  ret

; encodes a string using base64
; user is responsible for freeing the encoded string
; @param  rdi: string to encode
; @return rax: pointer to the encoded string
base64_encode_str:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp]   -> pointer to the string to encode

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

  mov   rdi, [rsp]
  call  strlen
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, rax
  call  base64_encode
  cmp   rax, 0
  jl    .error

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

base64_decode:
  ret

base64_decode_str:
  ret

base64_cust_encode:
  ret

; encodes a 3x8bit group from src to destination
; @param  rdi: decoded src
; @param  rsi: encoded destination
; @return rax: return code
_base64_encode_chunk:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> pointer to the src
  ; [rsp+0x8] -> pointer to the dest

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; get the first 6-bit of the first byte
  xor   rax, rax
  mov   rdi, [rsp]
  mov   al, byte [rdi]
  shr   rax, 0x2

  ; get the character from that value
  mov   rdi, rax
  call  _base64_get_char
  cmp   rax, 0
  jl    .error

  ; add character to the dest
  mov   rdi, [rsp+0x8]
  stosb

  mov   [rsp+0x8], rdi

  ; get the last 2-bit of the first byte
  xor   rax, rax
  mov   rdi, [rsp]
  mov   al, byte [rdi]
  and   rax, 0x3
  ; these bits are the high bit of the 2nd chunk
  shl   al, 0x4

  ; get the first 4-bit of the second byte
  xor   rbx, rbx
  mov   bl, byte [rdi+1]
  shr   bl, 0x4

  or    al, bl
  mov   rdi, rax
  call  _base64_get_char
  cmp   rax, 0
  jl    .error

  ; add character to the dest
  mov   rdi, [rsp+0x8]
  stosb

  mov   [rsp+0x8], rdi

  ; get the last 4-bit of the second byte
  xor   rax, rax
  mov   rdi, [rsp]
  mov   al, byte [rdi+1]
  and   al, 0xF
  ; these bits are the high bit of the 3rd chunk
  shl   al, 0x2

  ; get the first 2-bit of the third byte
  xor   rbx, rbx
  mov   bl, byte [rdi+2]
  shr   bl, 0x6

  or    al, bl
  mov   rdi, rax
  call  _base64_get_char
  cmp   rax, 0
  jl    .error

  ; add character to the dest
  mov   rdi, [rsp+0x8]
  stosb

  mov   [rsp+0x8], rdi

  ; get the last 6-bit of the third byte
  xor   rax, rax
  mov   rdi, [rsp]
  mov   al, byte [rdi+2]
  and   al, 0x3F

  mov   rdi, rax
  call  _base64_get_char
  cmp   rax, 0
  jl    .error

  ; add character to the dest
  mov   rdi, [rsp+0x8]
  stosb

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; returns the base64 character from the standard alphabet
; @param  rdi: index
; @return rax: base64 character
_base64_get_char:
  cmp   rdi, 0
  jl    .error

  cmp   rdi, BASE64_MAX_INDEX
  jg    .error

  xor   rax, rax
  mov   al, byte [base64_std_alphabet+rdi]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; return the size of the encoded string from the number of bytes
; to encode
; @param  rdi: number of bytes to encode
; @return rax: size of encoded string
_base64_get_encoded_size:
  cmp   rdi, 0
  jle   .error

  add   rdi, 2  ; integer arithmetic for ceil

  xor   rdx, rdx
  mov   rax, rdi
  mov   rbx, 3
  div   rbx

  xor   rdx, rdx
  mov   rbx, 4
  mul   rbx

  ; add one for null char
  inc   rax

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

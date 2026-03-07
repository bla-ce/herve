section .data

base64_std_alphabet db "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    db "abcdefghijklmnopqrstuvwxyz"
                    db "0123456789+/"

base64_dec_table  times 256 db BASE64_INVALID_CHAR

BASE64_MAX_INDEX  equ 63

BASE64_DEFAULT_ALPHABET equ base64_std_alphabet
BASE64_DEFAULT_PADCHAR  equ EQUAL_SIGN
BASE64_DEFAULT_PADDING  equ 1

BASE64_INVALID_CHAR equ 0xFF

base64_alphabet dq BASE64_DEFAULT_ALPHABET
base64_padchar  db BASE64_DEFAULT_PADCHAR
base64_padding  db BASE64_DEFAULT_PADDING

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
  call  _base64_get_encoded_char
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
  call  _base64_get_encoded_char
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
  call  _base64_get_encoded_char
  cmp   rax, 0
  jl    .error

  ; add character to the dest
  mov   rdi, [rsp+0x20]
  stosb

  cmp   byte [base64_padding], FALSE
  je    .end

  ; add pad char
  mov   al, byte [base64_padchar]
  stosb

  mov   [rsp+0x20], rdi
  jmp   .end

.padding_1:
  cmp   byte [base64_padding], FALSE
  je    .end

  mov   rdi, [rsp+0x20]
  mov   al, byte [base64_padchar]
  mov   rcx, 2
  rep   stosb

  mov   [rsp+0x20], rdi

.end:
  ; add null char at the end
  mov   rdi, [rsp+0x20]
  mov   byte [rdi], NULL_CHAR

  ; reset alphabet and padding setting
  call  _base64_reset

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

; decodes a base64 encoded data
; ad the original data can be a null-terminated string or arbitrary
; binary, the length of the decoded data will be stored at the
; address pointer to by rsi.
; user is responsible for freeing the decoded sequence of byte
; @param  rdi: pointer to the encoded string
; @param  rsi: address that will get the length of the decoded str
; @return rax: pointer to decoded bytes
base64_decode:
  sub   rsp, 0x28

  ; STACK USAGE
  ; [rsp]       -> pointer to the encoded string
  ; [rsp+0x8]   -> address receiving the length of the decoded str
  ; [rsp+0x10]  -> size of the decoded bytes
  ; [rsp+0x18]  -> pointer to the decoded bytes
  ; [rsp+0x20]  -> dynamic pointer

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x18], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; init decoded len to 0
  mov   qword [rsi], 0

  ; get size of the encoded string
  mov   rdi, [rsp]
  call  strlen
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; get size of decoded string
  mov   rdi, [rsp]
  call  _base64_get_decoded_size
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   [rdi], rax

  mov   rdi, rax
  inc   rdi   ; for potential null char
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax
  mov   [rsp+0x20], rax

  ; init decode table
  call  _base64_init_dec_table
  cmp   rax, 0
  jl    .error

.loop:
  cmp   qword [rsp+0x10], 0
  je    .loop_end

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x20]
  call  _base64_decode_quantum
  cmp   rax, 0
  jl    .error

  ; dec number of bytes to decode
  sub   qword [rsp+0x10], 4

  ; go past the decoded part
  mov   rdi, [rsp+0x20]
  add   rdi, 3
  mov   [rsp+0x20], rdi

  mov   rdi, [rsp]
  add   rdi, 4
  mov   [rsp], rdi

  jmp   .loop
.loop_end:
  ; reset alphabet and padding setting
  call  _base64_reset

  mov   rax, [rsp+0x18]

  jmp   .return

.error:
  mov   rdi, [rsp+0x18]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  ; set decoded length to 0
  mov   rdi, [rsp+0x8]
  mov   qword [rdi], 0

  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x28
  ret

; decodes base64 encoded data assuming that it was originally a
; null-terminated string
; user is responsible for freeing the decoded string
; @param  rdi: pointer to the encoded string
; @return rax: pointer to the decoded string
base64_decode_str:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> pointer to the encoded string
  ; [rsp+0x8]   -> length of the decoded string
  ; [rsp+0x10]  -> pointer to the decoded string

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

  mov   rdi, [rsp]
  lea   rsi, [rsp+0x8]
  call  base64_decode
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; add null char at the end of the string
  mov   rdi, rax
  add   rdi, [rsp+0x8]
  mov   byte [rdi], NULL_CHAR

  mov   rax, [rsp+0x10]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret


; sets the alphabet for the next base64 encode/decode call
; @param  rdi: pointer to the alphabet
; @return rax: return code
base64_set_alphabet:
  cmp   rdi, 0
  jle   .error

  mov   qword [base64_alphabet], rdi

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the padding character for the next base64 encode/decode call
; @param  rdi: new pad char
; @return rax: return code
base64_set_padchar:
  cmp   rdi, 0
  jle   .error

  mov   byte [base64_padchar], dil

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; enables the padding for the next base64 encode/decode call
; @return rax: return code
base64_enable_padding:
  mov   byte [base64_padding], TRUE

  mov   rax, SUCCESS_CODE
  ret

; disables the padding for the next base64 encode/decode call
; @return rax: return code
base64_disable_padding:
  mov   byte [base64_padding], FALSE

  mov   rax, SUCCESS_CODE
  ret

; resets the settings for base64 encoding/decoding
_base64_reset:
  mov   qword [base64_alphabet], BASE64_DEFAULT_ALPHABET
  mov   qword [base64_padchar], BASE64_DEFAULT_PADCHAR
  mov   qword [base64_padding], BASE64_DEFAULT_PADDING

  ret

; decodes a 4-6bit quantum from src to destination
; @param  rdi: encoded src
; @param  rsi: decoded destination
; @return rax: number of bytes added
_base64_decode_quantum:
  sub   rsp, 0x38

  ; STACK USAGE
  ; [rsp]       -> pointer to the src
  ; [rsp+0x8]   -> pointer to the dest
  ; [rsp+0x10]  -> first index
  ; [rsp+0x18]  -> second index
  ; [rsp+0x20]  -> third index
  ; [rsp+0x28]  -> fourth index
  ; [rsp+0x30]  -> concatenation of indices

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x10], 0
  mov   qword [rsp+0x18], 0
  mov   qword [rsp+0x20], 0
  mov   qword [rsp+0x28], 0
  mov   qword [rsp+0x30], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   rax, [rsp]
  mov   rdi, [rax]
  call  _base64_get_decoded_char
  cmp   rax, 0
  jl    .error

  shl   rax, 0x12

  mov   [rsp+0x10], rax

  mov   rax, [rsp]
  mov   rdi, [rax+0x1]
  call  _base64_get_decoded_char
  cmp   rax, 0
  jl    .error

  shl   rax, 0xC

  mov   [rsp+0x18], rax

  mov   rax, [rsp]
  mov   rdi, [rax+0x2]
  call  _base64_get_decoded_char
  cmp   rax, 0
  jl    .error

  shl   rax, 0x6

  mov   [rsp+0x20], rax

  mov   rax, [rsp]
  mov   rdi, [rax+0x3]
  call  _base64_get_decoded_char
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x28], rax

  xor   rbx, rbx
  mov   rax, [rsp+0x10]
  or    rbx, rax
  mov   rax, [rsp+0x18]
  or    rbx, rax
  mov   rax, [rsp+0x20]
  or    rbx, rax
  mov   rax, [rsp+0x28]
  or    rbx, rax

  ; swap bytes
  bswap ebx
  shr   ebx, 0x8

  mov   qword [rsp+0x30], rbx

  ; add three bytes to destination
  mov   rdi, [rsp+0x8]
  lea   rsi, [rsp+0x30]
  mov   rcx, 3
  rep   movsb

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x38
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
  call  _base64_get_encoded_char
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
  call  _base64_get_encoded_char
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
  call  _base64_get_encoded_char
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
  call  _base64_get_encoded_char
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

; returns the base64 character from the index
; @param  rdi: index
; @return rax: base64 character
_base64_get_encoded_char:
  cmp   rdi, 0
  jl    .error

  cmp   rdi, BASE64_MAX_INDEX
  jg    .error

  xor   rax, rax
  mov   rsi, [base64_alphabet]
  mov   al, byte [rsi+rdi]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the decoded character from the index
; @param  rdi: index
; @return rax: base64 character
_base64_get_decoded_char:
  movzx rdi, dil

  cmp   rdi, 0
  jl    .error

  cmp   rdi, 256
  jg    .error

  xor   rax, rax
  mov   al, byte [base64_dec_table+rdi]

  cmp   al, BASE64_INVALID_CHAR
  jne   .return

  xor   al, al

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret


; return the size of the decoded string from the encoded bytes
; @param  rdi: pointer to the encoded bytes
; @return rax: size of encoded string
_base64_get_decoded_size:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> pointer to the encoded bytes
  ; [rsp+0x8] -> number of pad char

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

  ; get the number of pad char
  mov   rdi, [rsp]
  mov   rsi, [base64_padchar]
  call  count_char
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; get size of the encoded bytes
  mov   rdi, [rsp]
  call  strlen
  cmp   rax, 0
  jl    .error

  xor   rdx, rdx
  mov   rbx, 4
  div   rbx

  cmp   rdx, 0
  je    .padding

  mov   rcx, rdx

  xor   rdx, rdx
  mov   rbx, 3
  mul   rbx

  ; check remainder to know the size of final group
  xor   rbx, rbx
  cmp   rcx, 2
  mov   rdx, 1
  cmove rbx, rdx

  cmp   rcx, 3
  mov   rdx, 2
  cmove rbx, rdx

  cmp   rcx, 1
  je    .error

  add   rax, rbx

  jmp   .return

.padding:
  xor   rdx, rdx
  mov   rbx, 3
  mul   rbx

  sub   rax, qword [rsp+0x8]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; initialises the decoding table with current alphabet
; @return rax: return code
_base64_init_dec_table:
  mov   rdi, base64_dec_table
  mov   al, BASE64_INVALID_CHAR
  mov   rcx, 256
  rep   stosb

  xor   rcx, rcx

.loop:
  xor   rax, rax
  mov   rdi, [base64_alphabet]
  mov   al, byte [rdi+rcx]

  mov   byte [base64_dec_table+rax], cl

  inc   rcx
  cmp   rcx, BASE64_MAX_INDEX
  jg    .loop_end

  jmp   .loop

.loop_end:
  mov   rax, SUCCESS_CODE
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

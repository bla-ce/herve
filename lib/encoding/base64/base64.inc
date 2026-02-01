%include "base64_errors.inc"

section .data
  base64_table:
    db 0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80
    db 0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80
    db 0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,62  ,0x80,0x80,0x80,63
    db 52  ,53  ,54  ,55  ,56  ,57  ,58  ,59  ,60  ,61  ,0x80,0x80,0x80,0x80,0x80,0x80
    db 0x80, 0  , 1  , 2  , 3  , 4  , 5  , 6  , 7  , 8  , 9  ,10  ,11  ,12  ,13  ,14
    db 15  ,16  ,17  ,18  ,19  ,20  ,21  ,22  ,23  ,24  ,25  ,0x80,0x80,0x80,0x80,0x80
    db 0x80,26  ,27  ,28  ,29  ,30  ,31  ,32  ,33  ,34  ,35  ,36  ,37  ,38  ,39  ,40
    db 41  ,42  ,43  ,44  ,45  ,46  ,47  ,48  ,49  ,50  ,51  ,0x80,0x80,0x80,0x80,0x80
    db 0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80
    db 0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80
    db 0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80
    db 0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80
    db 0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80
    db 0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80
    db 0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80
    db 0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80
  base_64_table_end:

  base64_enc_table:
    db 'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P'
    db 'Q','R','S','T','U','V','W','X','Y','Z','a','b','c','d','e','f'
    db 'g','h','i','j','k','l','m','n','o','p','q','r','s','t','u','v'
    db 'w','x','y','z','0','1','2','3','4','5','6','7','8','9','+','/'
  base64_enc_table_end:

  BASE_64_MAX_VALUE db 63

section .text
; encodes base64 string
; developer is responsible of freeing the string
; @param  rdi: pointer to null-terminated string
; @return rax: pointer to encoded string
base64_encode:
  sub   rsp, 0x40

  ; STACK USAGE
  ; [rsp]       -> pointer to the string
  ; [rsp+0x8]   -> length of the string
  ; [rsp+0x10]  -> pointer to the allocated encoded string
  ; [rsp+0x18]  -> dynamic pointer of the above
  ; [rsp+0x20]  -> dynamic first character
  ; [rsp+0x28]  -> dynamic second character
  ; [rsp+0x30]  -> dynamic third character
  ; [rsp+0x38]  -> encoded character

  mov   [rsp], rdi
  mov   qword [rsp+0x10], 0
  mov   qword [rsp+0x20], NULL_CHAR
  mov   qword [rsp+0x28], NULL_CHAR
  mov   qword [rsp+0x30], NULL_CHAR

  mov   qword [base64_errno], BASE64_INVALID_ARG

  cmp   rdi, 0
  jle   .error

  ; get length of the string
  mov   rdi, [rsp]
  call  strlen
  cmp   rax, 0
  jl    .error

  add   rax, rax

  mov   qword [rsp+0x8], rax

  mov   rdi, qword [rsp+0x8]
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax
  mov   [rsp+0x18], rax

  xor   r9, r9

.loop:
  mov   rsi, [rsp]

.get_first_char:
  ; get first char
  mov   dil, byte [rsi+r9]
  movzx rdi, dil

  test  rdi, rdi
  jz    .end_loop    ; padding 2

  mov   [rsp+0x20], rdi
  inc   r9

  ; get second char
  mov   rsi, [rsp]
  mov   dil, byte [rsi+r9]
  movzx rdi, dil

  inc   r9

  test  rdi, rdi
  jz    .convert    ; padding 2

  mov   [rsp+0x28], rdi

  ; get third char
  mov   rsi, [rsp]
  mov   dil, byte [rsi+r9]
  movzx rdi, dil

  inc   r9

  test  rdi, rdi
  jz     .convert    ; padding 1

  mov   [rsp+0x30], rdi

.convert:
  ; convert 3*8bit to 4*6bit
  mov   rax, qword [rsp+0x20]
  shl   rax, 16
  mov   qword [rsp+0x20], rax

  mov   rax, qword [rsp+0x28]
  shl   rax, 8
  mov   qword [rsp+0x28], rax

  mov   rax, qword [rsp+0x20]
  or    rax, qword [rsp+0x28]
  or    rax, qword [rsp+0x30]

  mov   qword [rsp+0x38], rax

  ; extract first 6-bit char
  mov   rax, qword [rsp+0x38]
  shr   rax, 18
  and   rax, 0x3F
  mov   rdi, rax
  call  encode_base64_char

  mov   qword [base64_errno], BASE64_ENCODING_ERR
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x18]
  stosb
  mov   [rsp+0x18], rdi

  ; extract second 6-bit char
  mov   rax, qword [rsp+0x38]
  shr   rax, 12
  and   rax, 0x3F
  mov   rdi, rax
  call  encode_base64_char

  mov   qword [base64_errno], BASE64_ENCODING_ERR
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x18]
  stosb
  mov   [rsp+0x18], rdi

  ; extract third 6-bit char
  mov   rax, qword [rsp+0x38]
  shr   rax, 6
  test  al, al
  jz    .padding_2

  and   rax, 0x3F
  mov   rdi, rax
  call  encode_base64_char

  mov   qword [base64_errno], BASE64_ENCODING_ERR
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x18]
  stosb
  mov   [rsp+0x18], rdi

  ; extract fourth 6-bit char
  mov   rax, qword [rsp+0x38]
  and   rax, 0x3F
  test  al, al
  jz    .padding_1

  mov   rdi, rax
  call  encode_base64_char

  mov   qword [base64_errno], BASE64_ENCODING_ERR
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x18]
  stosb
  mov   [rsp+0x18], rdi

  ; reset values
  mov   qword [rsp+0x20], NULL_CHAR
  mov   qword [rsp+0x28], NULL_CHAR
  mov   qword [rsp+0x30], NULL_CHAR

  jmp   .loop

.padding_2:
  mov   rdi, [rsp+0x18]
  mov   rax, EQUAL_SIGN
  mov   rcx, 2
  rep   stosb
  mov   [rsp+0x18], rdi

  jmp   .end_loop

.padding_1:
  mov   rdi, [rsp+0x18]
  mov   rax, EQUAL_SIGN
  stosb
  mov   [rsp+0x18], rdi

.end_loop:
  ; make sure the string is NULL terminated
  mov   rdi, [rsp+0x18]
  mov   rax, NULL_CHAR
  stosb

  mov   qword [base64_errno], BASE64_NO_ERR
  mov   rax, [rsp+0x10]

  jmp   .return

.error:
  mov   rdi, [rsp+0x10]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x40
  ret

; decodes base64 encoded string, developer is responsible of freeing the string
; @param  rdi: Pointer to a NULL-terminated Base64 encoded string.
; @return rax: Pointer to the newly allocated decoded string (or FAILURE_CODE)
base64_decode:
  sub   rsp, 0x48

  mov   qword [base64_errno], BASE64_INVALID_ARG

  ; STACK USAGE
  ; [rsp]       -> pointer to the string
  ; [rsp+0x8]   -> length of the string
  ; [rsp+0x10]  -> pointer to the allocated encoded string
  ; [rsp+0x18]  -> dynamic pointer of the above
  ; [rsp+0x20]  -> dynamic first 6-bit
  ; [rsp+0x28]  -> dynamic second 6-bit
  ; [rsp+0x30]  -> dynamic third 6-bit
  ; [rsp+0x38]  -> dynamic fourth 6-bit
  ; [rsp+0x40]  -> decoded character

  mov   [rsp], rdi
  mov   qword [rsp+0x10], 0

  cmp   rdi, 0
  jle   .error

  ; get length of the string
  mov   rdi, [rsp]
  call  strlen
  cmp   rax, 0
  jl    .error

  add   rax, rax

  mov   qword [rsp+0x8], rax

  ; malloc same length string
  mov   rdi, rax
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax
  mov   [rsp+0x18], rax

  xor   r9, r9

.loop:
  mov   rsi, [rsp]

  ; get first char
  mov   dil, byte [rsi+r9]
  movzx rdi, dil

  test  rdi, rdi
  jz    .end_loop

  ; get 6 bit representation
  call  decode_base64_char

  mov   qword [base64_errno], BASE64_DECODING_ERR
  cmp   rax, 0
  jl    .error

  ; save 6bit
  mov   [rsp+0x20], rax

  inc r9

  ; get second char
  mov   rsi, [rsp]
  mov   dil, byte [rsi+r9]
  movzx rdi, dil

  test  rdi, rdi
  jz    .end_loop

  ; get 6 bit representation
  call  decode_base64_char

  mov   qword [base64_errno], BASE64_DECODING_ERR
  cmp   rax, 0
  jl    .error

  ; save 6bit
  mov   [rsp+0x28], rax

  inc   r9

  ; get third char
  mov   rsi, [rsp]
  mov   dil, byte [rsi+r9]
  movzx rdi, dil

  test  rdi, rdi
  jz    .end_loop

  ; get 6 bit representation
  call  decode_base64_char

  mov   qword [base64_errno], BASE64_DECODING_ERR
  cmp   rax, 0
  jl    .error

  ; save 6bit
  mov   [rsp+0x30], rax

  inc   r9

  ; get fourth char
  mov   rsi, [rsp]
  mov   dil, byte [rsi+r9]
  movzx rdi, dil

  test  rdi, rdi
  jz    .end_loop

  ; get 6 bit representation
  call  decode_base64_char

  mov   qword [base64_errno], BASE64_DECODING_ERR
  cmp   rax, 0
  jl    .error

  ; save 6bit
  mov   [rsp+0x38], rax

  inc   r9

  ; convert 4*6bit to 3*8bit
  mov   rax, qword [rsp+0x20]
  shl   rax, 18
  mov   qword [rsp+0x20], rax

  mov   rax, qword [rsp+0x28]
  shl   rax, 12
  mov   qword [rsp+0x28], rax

  mov   rax, qword [rsp+0x30]
  shl   rax, 6
  mov   qword [rsp+0x30], rax

  mov   rax, qword [rsp+0x20]
  or    rax, qword [rsp+0x28]
  or    rax, qword [rsp+0x30]
  or    rax, qword [rsp+0x38]

  ; save triple
  mov   qword [rsp+0x40], rax

  ; check padding
  ; get last char and check if equal
  mov   rax, qword [rsp+0x38]
  cmp   rax, 0x80
  je    .padding_1

  ; extract first 8-bit char
  mov   rax, qword [rsp+0x40]
  shr   rax, 16
  and   rax, 0xFF

  ; move to decoded string
  mov   rdi, [rsp+0x18]
  stosb

  mov   [rsp+0x18], rdi

  ; extract second 8-bit char
  mov   rax, qword [rsp+0x40]
  shr   rax, 8
  and   rax, 0xFF
  jz    .padding_1

  ; move to decoded string
  mov   rdi, [rsp+0x18]
  stosb

  mov   [rsp+0x18], rdi

  ; extract third 8-bit char
  mov   rax, qword [rsp+0x40]
  and   rax, 0xFF

  ; move to decoded string
  mov   rdi, [rsp+0x18]
  stosb

  mov   [rsp+0x18], rdi

  jmp   .loop

.padding_1:
  ; check if padding is 2 long
  mov   rax, qword [rsp+0x30]
  shr   rax, 6
  cmp   rax, 0x80
  je    .padding_2

  ; extract first 8-bit char
  mov   rax, qword [rsp+0x40]
  shr   rax, 16
  and   rax, 0xFF

  ; move to decoded string
  mov   rdi, [rsp+0x18]
  stosb

  ; extract first 8-bit char
  mov   rax, qword [rsp+0x40]
  shr   rax, 8
  and   rax, 0xFF

  ; move to decoded string
  stosb

  mov   [rsp+0x18], rdi

  jmp   .end_loop

.padding_2:
  ; extract first 8-bit char
  mov   rax, qword [rsp+0x40]
  shr   rax, 16
  and   rax, 0xFF

  ; move to decoded string
  mov   rdi, [rsp+0x18]
  stosb

  mov   [rsp+0x18], rdi

  jmp   .end_loop

.end_loop:
  mov   rdi, [rsp+0x18]
  mov   rax, NULL_CHAR
  stosb

  mov   rax, [rsp+0x10]

  mov   qword [base64_errno], BASE64_NO_ERR
  jmp   .return

.error:
  mov   rdi, [rsp+0x10]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x48
  ret

; converts a single Base64 character into its corresponding 6-bit integer value
; @param  rdi: char
; @return rax: 6bit
decode_base64_char:
  mov   qword [base64_errno], BASE64_INVALID_CHAR

  cmp   rdi, 0
  jl    .error

  cmp   rdi, 0xFF
  jg    .error

  ; get value from base 64 table
  mov   al, byte [base64_table+rdi]
  movzx rax, al

  mov   qword [base64_errno], BASE64_NO_ERR
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; converts a 6-bit integer value into its correspoinding Base64 character
; @param  rdi: 6bit
; @return rax: char
encode_base64_char:
  mov   qword [base64_errno], BASE64_INVALID_CHAR

  cmp   rdi, 0
  jl    .error

  cmp   rdi, BASE_64_MAX_VALUE
  jg    .equal

  mov   al, byte [base64_enc_table+rdi]
  movzx rax, al

  mov   qword [base64_errno], BASE64_NO_ERR
  jmp   .return

.equal:
  mov   rax, EQUAL_SIGN

  mov   qword [base64_errno], BASE64_NO_ERR
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

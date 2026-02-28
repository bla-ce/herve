section .data

hex_table db "0123456789abcdef"

HEX_ENCODING_MAX_BYTE equ 8 ; max 8 bytes for each nencode

section .text

; encode a byte to hex to the destination
; @param  rdi: byte to encode
; @param  rsi: pointer to the destination
; @return rax: return code
hex_encode:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> bytes to encode
  ; [rsp+0x8] -> pointer to the destination

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  cmp   rsi, 0
  jle   .error

  xor   rdi, rdi
  mov   dil, byte [rsp]
  shr   rdi, 4

  mov   rax, [hex_table+rdi]

  mov   rdi, [rsp+0x8]
  stosb

  xor   rdi, rdi
  mov   dil, byte [rsp]
  and   rdi, 0x0F

  mov   rax, [hex_table+rdi]

  mov   rdi, [rsp+0x8]
  inc   rdi
  stosb

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; encode n bytes to hex to the destination
; @param  rdi: bytes to encode
; @param  rsi: pointer to the destination
; @param  rdx: number of byte to encode
; @return rax: return code
hex_nencode:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> bytes to encode
  ; [rsp+0x8]   -> pointer to the destination
  ; [rsp+0x10]  -> number of bytes to encode

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx

  cmp   rsi, 0
  jle   .error

.loop:
  cmp   qword [rsp+0x10], 0
  jle   .loop_end

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  call  hex_encode
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  shr   rdi, 8  ; get next byte
  mov   [rsp], rdi

  mov   rsi, [rsp+0x8]
  add   rsi, 2
  mov   [rsp+0x8], rsi

  dec   qword [rsp+0x10]

  jmp   .loop

.loop_end:
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

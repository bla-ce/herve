%include "uuid_v4.inc"

section .data

RDRAND_CPUID_POS    equ 30
RDRAND_RETRY_LIMIT  equ 10

; bit shift positions for uuid segment
UUID_SHR_BYTES_0_3    equ 0
UUID_SHR_BYTES_4_5    equ 32
UUID_SHR_BYTES_6_7    equ 48
UUID_SHR_BYTES_10_15  equ 16

UUID_STR_LEN equ 37

section .text

; converts first and second 64bit registers to a uuid string
; representation
; user is responsible for freeing the string
; @param  rdi: first 64-bit
; @param  rsi: second 64-bit
; @return rax: pointer to the string
uuid_to_str:
  sub   rsp, 0x20

  ; STACK USAGE
  ; [rsp]       -> first 64 bit
  ; [rsp+0x8]   -> second 64 bit
  ; [rsp+0x10]  -> pointer to the string
  ; [rsp+0x18]  -> dynamic pointer to the string

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x10], 0
  mov   qword [rsp+0x18], 0

  mov   rdi, UUID_STR_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax
  mov   [rsp+0x18], rax

  ; encode 0-3
  mov   rdi, [rsp]
  shr   rdi, UUID_SHR_BYTES_0_3
  mov   rsi, [rsp+0x18]
  mov   rdx, 4
  call  hex_nencode
  cmp   rax, 0
  jl    .error

  ; add dash
  mov   rdi, [rsp+0x10]
  add   rdi, 8
  mov   rax, HYPHEN
  stosb

  mov   [rsp+0x18], rdi

  ; encode 4-5
  mov   rdi, [rsp]
  shr   rdi, UUID_SHR_BYTES_4_5
  mov   rsi, [rsp+0x18]
  mov   rdx, 2
  call  hex_nencode
  cmp   rax, 0
  jl    .error

  ; add dash
  mov   rdi, [rsp+0x18]
  add   rdi, 4
  mov   rax, HYPHEN
  stosb

  mov   [rsp+0x18], rdi

  ; encode 6-7
  mov   rdi, [rsp]
  shr   rdi, UUID_SHR_BYTES_6_7
  mov   rsi, [rsp+0x18]
  mov   rdx, 2
  call  hex_nencode
  cmp   rax, 0
  jl    .error

  ; add dash
  mov   rdi, [rsp+0x18]
  add   rdi, 4
  mov   rax, HYPHEN
  stosb

  mov   [rsp+0x18], rdi

  ; endode 8-9
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x18]
  mov   rdx, 2
  call  hex_nencode
  cmp   rax, 0
  jl    .error

  ; add dash
  mov   rdi, [rsp+0x18]
  add   rdi, 4
  mov   rax, HYPHEN
  stosb

  mov   [rsp+0x18], rdi

  ; encode 10-15
  mov   rdi, [rsp+0x8]
  shr   rdi, UUID_SHR_BYTES_10_15
  mov   rsi, [rsp+0x18]
  mov   rdx, 6
  call  hex_nencode
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x18]
  add   rdi, 12
  mov   rax, NULL_CHAR
  stosb

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
  add   rsp, 0x20
  ret

; returns whether RDRAND is supported with this CPU or not
; @return rax: RDRAND is supported (1 YES, 0 NO)
uuid_rdrand_is_supported:
  mov   rax, 1
  cpuid

  shr   rcx, RDRAND_CPUID_POS
  and   rcx, 1

  mov   rax, rcx

  ret

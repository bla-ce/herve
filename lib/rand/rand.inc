section .data

RDRAND_CPUID_POS    equ 30
RDRAND_RETRY_LIMIT  equ 10

RDRAND_ERRNO dq 0

section .text

; returns whether RDRAND is supported with this CPU or not
; @return rax: RDRAND is supported (1 YES, 0 NO)
rand_rdrand_is_supported:
  mov   rax, 1
  cpuid

  shr   rcx, RDRAND_CPUID_POS
  and   rcx, 1

  mov   rax, rcx

  ret

; generates a random number using the rdrand instruction
; if the operations fails RDRAND_ERRNO is set
; @return rax: generated number
rand_generate_number:
  ; check if RDRAND is supported by the CPU
  call  rand_rdrand_is_supported
  cmp   rax, 0
  je    .error

  xor   rcx, rcx
.loop:
  rdrand  rax
  jc    .return

  inc   rcx

  cmp   rcx, RDRAND_RETRY_LIMIT
  jge   .error

  jmp   .loop

.error:
  mov   qword [RDRAND_ERRNO], TRUE

.return:
  mov   qword [RDRAND_ERRNO], FALSE
  ret

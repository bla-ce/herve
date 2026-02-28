section .data

; masks to set the versions and variants
UUIDV4_VARIANT_OR_MASK  dq 0x80
UUIDV4_VARIANT_AND_MASK dq 0xFFFFFFFFFFFFFF3F

UUIDV4_VERSION_OR_MASK  dq 0x0040000000000000
UUIDV4_VERSION_AND_MASK dq 0xFF0FFFFFFFFFFFFF

section .text

; generates a v4 uuid
; user is responsible for freeing the uuid string
; @return rax: pointer to the generated uuid
uuid_v4:
  ; check if RDRAND is supported by the CPU
  call  uuid_rdrand_is_supported
  cmp   rax, 0
  je    .error

  ; get first 64bit random number
  xor   rcx, rcx
.loop1:
  rdrand  rdi
  jc    .loop1_end

  inc   rcx

  cmp   rcx, RDRAND_RETRY_LIMIT
  jge   .error

  jmp   .loop1
.loop1_end:

  ; get second 64bit random number
  xor   rcx, rcx
.loop2:
  rdrand  rsi
  jc    .loop2_end

  inc   rcx

  cmp   rcx, RDRAND_RETRY_LIMIT
  jge   .error

  jmp   .loop2
.loop2_end:

  ; set the version
  and   rdi, qword [UUIDV4_VERSION_AND_MASK]
  or    rdi, qword [UUIDV4_VERSION_OR_MASK]

  ; set the variant
  and   rsi, qword [UUIDV4_VARIANT_AND_MASK]
  or    rsi, qword [UUIDV4_VARIANT_OR_MASK]

  ; to string - registers are already set
  call  uuid_to_str
  cmp   rax, 0
  jl    .error

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

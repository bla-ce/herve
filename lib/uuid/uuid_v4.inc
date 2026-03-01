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
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> first 64-bit generated number
  ; [rsp+0x8] -> first 64-bit generated number

  call  rand_generate_number
  cmp   qword [RDRAND_ERRNO], TRUE
  je    .error

  mov   [rsp], rax

  call  rand_generate_number
  cmp   qword [RDRAND_ERRNO], TRUE
  je    .error

  mov   [rsp+0x8], rax

  ; set the version
  mov   rdi, [rsp]
  and   rdi, qword [UUIDV4_VERSION_AND_MASK]
  or    rdi, qword [UUIDV4_VERSION_OR_MASK]

  ; set the variant
  mov   rsi, [rsp+0x8]
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
  add   rsp, 0x10
  ret

section .bss

blowfish_state_t:
blowfish_state_t_end:

section .data

BCRYPT_DEFAULT_COST   equ 10
BCRYPT_MIN_COST       equ 4
BCRYPT_MAX_COST       equ 31

BCRYPT_HASH_SIZE    equ 61  ; include null char
BCRYPT_MAX_PWD_LEN  equ 73  ; include null char

bcrypt_magic_string db "OrpheanBeholderScryDoubt" ; no need for null char

section .text

; hashes a password using bcrypt algorithm
; user is responsible for freeing the hash
; @param  rdi: pointer to the password
; @param  rsi: cost
; @return rax: pointer to the hash
bcrypt_hash:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> pointer to the password
  ; [rsp+0x8]   -> cost
  ; [rsp+0x10]  -> random salt
  ; [rsp+0x18]  -> blowfish state struct
  ; [rsp+0x20]  -> duplicated magic string
  ; [rsp+0x28]  -> pointer to the hash

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x10], 0
  mov   qword [rsp+0x18], 0
  mov   qword [rsp+0x20], 0

  ; validate password
  mov   rdi, [rsp]
  call  bcrypt_validate_password
  cmp   rax, 0
  jl    .error

  ; validate cost
  mov   rdi, [rsp+0x8]
  call  bcrypt_validate_cost
  cmp   rax, 0
  je    .generate_salt

  mov   [rsp+0x8], BCRYPT_DEFAULT_SALT

.generate_salt:
  ; generate random 128-bit salt
  call  bcrypt_generate_salt
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x10]
  mov   rdx, [rsp+0x8]
  call  eks_blowfish_setup
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; copy magic string
  mov   rdi, bcrypt_magic_string
  call  strdup
  cmp   rax, 0
  jle   .error

  mov   [rsp+0x20], rax

  ; expensive loop
  mov   rdi, [rsp+0x18]
  mov   rsi, [rsp+0x20]
  call  bcrypt_encrypt_loop
  cmp   rax, 0
  jl    .error

  ; concatenate cost, salt and hash
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x10]
  mov   rdx, [rsp+0x20]
  call  bcrypt_concat_values
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x28], rax

  ; free salt
  mov   rdi, [rsp+0x10]
  call  free
  cmp   rax, 0
  jl    .error

  mov   qword [rdi+0x10], 0

  ; free blowfish state
  mov   rdi, [rsp+0x18]
  call  free
  cmp   rax, 0
  jl    .error

  mov   qword [rdi+0x8], 0

  mov   rax, [rsp+0x28]

  jmp   .return

.error:
  mov   rdi, [rsp+0x20]
  test  rdi, rdi
  jz    .free_salt

  call  free

.free_salt:
  mov   rdi, [rsp+0x10]
  test  rdi, rdi
  jz    .free_blowfish_struct

  call  free

.free_blowfish_struct:
  mov   rdi, [rsp+0x18]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret

; validates the current cost for bcrypt hashing
; @param  rdi: cost
; @return rax: return code
bcrypt_validate_cost:
  cmp   rdi, BCRYPT_MIN_COST
  jl    .error

  cmp   rdi, BCRYPT_MAX_COST
  jg    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; validates the password for bcrypt hashing
bcrypt_validate_password:
  cmp   rdi, 0
  jle   .error

  call  strlen
  cmp   rax, 0
  jl    .error
  cmp   rax, BCRYPT_MAX_PWD_LEN
  jge   .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; generates a 128-bit random salt
; user is responsible for freeing the salt
; @return   rax: pointer to the salt
bcrypt_generate_salt:
  ret

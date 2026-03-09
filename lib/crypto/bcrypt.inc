%include "blowfish_constants.inc"

section .bss

blowfish_state_t:
  .p_array  resb BCRYPT_P_ARRAY_SIZE
  .s_boxes  resb BCRYPT_S_BOXES_SIZE
blowfish_state_t_end:

section .data

BCRYPT_P_ARRAY_SIZE equ 72
BCRYPT_S_BOXES_SIZE equ 4096

BCRYPT_BLOWFISH_LEN equ blowfish_state_t_end - blowfish_state_t

BCRYPT_BLOWFISH_OFF_P_ARRAY equ 0x0
BCRYPT_BLOWFISH_OFF_S_BOXES equ BCRYPT_BLOWFISH_OFF_P_ARRAY + BCRYPT_P_ARRAY_SIZE

BCRYPT_DEFAULT_COST   equ 10
BCRYPT_MIN_COST       equ 4
BCRYPT_MAX_COST       equ 31

BCRYPT_HASH_SIZE    equ 61  ; include null char
BCRYPT_MAX_PWD_LEN  equ 73  ; include null char

bcrypt_magic_string db "OrpheanBeholderScryDoubt" ; no need for null char

bcrypt_base64_alphabet  db  "./ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                        db  "abcdefghijklmnopqrstuvwxyz"
                        db  "0123456789"

section .text

; hashes a password using bcrypt algorithm
; user is responsible for freeing the hash
; @param  rdi: pointer to the password
; @param  rsi: cost
; @return rax: pointer to the hash
bcrypt_hash:
  sub   rsp, 0x30

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
  call  _bcrypt_validate_password
  cmp   rax, 0
  jl    .error

  ; validate cost
  mov   rdi, [rsp+0x8]
  call  _bcrypt_validate_cost
  cmp   rax, 0
  je    .generate_salt

  mov   qword [rsp+0x8], BCRYPT_DEFAULT_COST

.generate_salt:
  ; generate random 128-bit salt
  call  _bcrypt_generate_salt
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x10]
  mov   rdx, [rsp+0x8]
  call  _bcrypt_eks_blowfish_setup
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
  call  _bcrypt_encrypt_loop
  cmp   rax, 0
  jl    .error

  ; concatenate cost, salt and hash
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x10]
  mov   rdx, [rsp+0x20]
  call  _bcrypt_concat_values
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

  ; free duplicated magic string
  mov   rdi, [rsp+0x20]
  call  free
  cmp   rax, 0
  jl    .error

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
  add   rsp, 0x30
  ret

; validates the current cost for bcrypt hashing
; @param  rdi: cost
; @return rax: return code
_bcrypt_validate_cost:
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
_bcrypt_validate_password:
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
; @return rax: pointer to the salt
_bcrypt_generate_salt:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]       -> first generated number
  ; [rsp+0x8]   -> second generated number

  ; generate first random 64-bit number
  call  rand_generate_number
  mov   [rsp], rax

  ; generate second random 64-bit number
  call  rand_generate_number
  mov   [rsp+0x8], rax

  ; set the alphabet for next base64 encoding
  mov   rdi, bcrypt_base64_alphabet
  call  base64_set_alphabet
  cmp   rax, 0
  jl    .error

  ; disable padding
  call  base64_disable_padding
  cmp   rax, 0
  jl    .error

  ; base64 encode 128-bit salt
  lea   rdi, [rsp]
  mov   rsi, 16 ; we need to encode 16 byte
  call  base64_encode
  cmp   rax, 0
  jl    .error

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; runs encryption loop on the magic string
; the string is modified in-place
; @param  rdi: pointer to the blowfish state
; @param  rsi: pointer to the duplicated magic string
; @return rax: return code
_bcrypt_encrypt_loop:
  ret

_bcrypt_encrypt_block:
  ret

; concatenates the cost, the salt and the encrypted string
; user is responsible for freeing the final hash
; @param  rdi: cost
; @param  rsi: pointer to the salt
; @param  rdx: pointer to the encrypted magic string
; @return rax: pointer to the hash
_bcrypt_concat_values:
  ret

; initialises the blowfish state with cost, salt, and password
; @param  rdi: pointer to the password
; @param  rsi: pointer to the salt
; @param  rdx: cost
; @return rax: pointer to the state struct
_bcrypt_eks_blowfish_setup:
  sub   rsp, 0x28

  ; STACK USAGE
  ; [rsp]       -> pointer to the password
  ; [rsp+0x8]   -> pointer to the salt
  ; [rsp+0x10]  -> cost
  ; [rsp+0x18]  -> pointer to the state struct
  ; [rsp+0x20]  -> counter

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   qword [rsp+0x18], 0
  mov   qword [rsp+0x20], 0

  call  _bcrypt_init_state
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; permute P and S based on the password and salt
  mov   rdi, [rsp+0x18]
  mov   rsi, [rsp]
  mov   rdx, [rsp+0x8]
  call  _bcrypt_expand_key
  cmp   rax, 0
  jl    .error

  ; we need to get 2^cost which is equal to 1 << cost
  mov   rcx, [rsp+0x10]
  mov   rax, 1
  shl   rax, cl

  mov   qword [rsp+0x20], rax

.loop:
  mov   rdi, [rsp+0x18]
  mov   rsi, NO_ARG
  mov   rdx, [rsp]
  call  _bcrypt_expand_key
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x18]
  mov   rsi, [rsp+0x8]
  mov   rdx, NO_ARG
  call  _bcrypt_expand_key
  cmp   rax, 0
  jl    .error

  dec   qword [rsp+0x20]
  cmp   qword [rsp+0x20], 0
  jle   .loop_end

  jmp   .loop
.loop_end:

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

; @param  rdi: pointer to the blowfish state struct
; @param  rsi: pointer to the password
; @param  rdx: pointer to the salt
; @return rax: return code
_bcrypt_expand_key:
  sub   rsp, 0x38

  ; STACK USAGE
  ; [rsp]       -> pointer to the blowfish state struct
  ; [rsp+0x8]   -> pointer to the password
  ; [rsp+0x10]  -> pointer to the salt
  ; [rsp+0x18]  -> length of the password
  ; [rsp+0x20]  -> pointer to the decoded base64 salt
  ; [rsp+0x28]  -> 64-bit block
  ; [rsp+0x30]  -> counter

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   qword [rsp+0x20], 0
  mov   qword [rsp+0x28], 0
  mov   qword [rsp+0x30], 0

  cmp   rdi, 0
  jle   .error

  ; verify salt before
  cmp   rdx, 0
  jl    .error

  cmp   rsi, 0
  jl    .error
  je    .loop_end

  ; get the length of the password
  mov   rdi, [rsp+0x8]
  call  strlen
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  xor   r9, r9    ; counter
  xor   r10, r10  ; password index to store

.loop:
  cmp   r9, BCRYPT_P_ARRAY_SIZE
  jge   .loop_end

  ; get the password byte at index r9
  mov   rdi, [rsp+0x8]
  add   rdi, r10
  mov   al, byte [rdi]

  mov   rdi, [rsp]
  add   rdi, r9
  xor   byte [rdi], al

  inc   r9
  inc   r10

  cmp   r10, [rsp+0x18]
  jne   .loop

  ; reset password
  xor   r10, r10

  jmp   .loop

.loop_end:
  cmp   qword [rsp+0x10], 0
  je    .encrypt_p_loop

  ; set the alphabet for next base64 decoding
  mov   rdi, bcrypt_base64_alphabet
  call  base64_set_alphabet
  cmp   rax, 0
  jl    .error

  ; disable padding
  call  base64_disable_padding
  cmp   rax, 0
  jl    .error

  ; decode base64 salt
  mov   rdi, [rsp+0x10]
  mov   rsi, NO_ARG
  call  base64_decode
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

.encrypt_p_loop:
  cmp   qword [rsp+0x30], 8
  jg    .encrypt_p_loop_end

  ; block = block xor saltHalf[n mod 2]
  ; get the right salt half
  xor   rdx, rdx
  mov   rax, qword [rsp+0x30]
  and   rax, 1
  mov   rbx, 8
  mul   rbx

  xor   rdx, rdx
  mov   rdi, [rsp+0x20]
  test  rdi, rdi
  jz    .xor_salt_1

  mov   rdx, [rdi+rax]

.xor_salt_1:
  xor   [rsp+0x28], rdx

  mov   rdi, [rsp]
  lea   rsi, [rsp+0x28]
  call  _bcrypt_encrypt_block
  cmp   rax, 0
  jl    .error

  ; get P-array
  mov   rdx, [rsp]

  ; get the offset 8n
  mov   rax, [rsp+0x30]
  lea   rdi, [rdx + rax * 0x8]

  ; move block to p-array
  mov   rax, [rsp+0x28]
  ror   rax, 32
  mov   [rdi], rax

  add   qword [rsp+0x30], 0x1

  jmp   .encrypt_p_loop
.encrypt_p_loop_end:

  ; reset counter
  mov   qword [rsp+0x30], 0

.encrypt_s_loop:
  cmp   qword [rsp+0x30], 511
  jg    .encrypt_s_loop_end

  ; block = block xor saltHalf[n mod 2]
  ; get the right salt half
  xor   rdx, rdx
  mov   rax, qword [rsp+0x30]
  and   rax, 1
  mov   rbx, 8
  mul   rbx

  xor   rdx, rdx
  mov   rdi, [rsp+0x20]
  test  rdi, rdi
  jz    .xor_salt_2

  mov   rdx, [rdi+rax]

.xor_salt_2:
  xor   [rsp+0x28], rdx

  mov   rdi, [rsp]
  lea   rsi, [rsp+0x28]
  call  _bcrypt_encrypt_block
  cmp   rax, 0
  jl    .error

  ; get S-boxes
  mov   rdx, [rsp]

  ; get the offset 8n
  mov   rax, [rsp+0x30]
  lea   rdi, [rdx + rax * 0x8 + BCRYPT_BLOWFISH_OFF_S_BOXES]

  ; move block to s-array
  mov   rax, [rsp+0x28]
  ror   rax, 32
  mov   [rdi], rax

  add   qword [rsp+0x30], 0x1

  jmp   .encrypt_s_loop
.encrypt_s_loop_end:
  mov   rdi, [rsp+0x20]
  test  rdi, rdi
  jz    .skip_free

  call  free
  mov   qword [rsp+0x20], 0
  cmp   rax, 0
  jl    .error

.skip_free:
  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rdi, [rsp+0x20]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x38
  ret

; initialises the state with const p and s values
; user is responsible for freeing the struct
; @return rax: pointer to the blowfish state struct
_bcrypt_init_state:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp]   -> pointer to the blowfish state struct

  mov   qword [rsp], 0

  mov   rdi, BCRYPT_BLOWFISH_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp], rax

  ; copy p-array
  mov   rdi, [rsp]
  add   rdi, BCRYPT_BLOWFISH_OFF_P_ARRAY
  mov   rsi, blowfish_const_p
  mov   rcx, BCRYPT_P_ARRAY_SIZE
  rep   movsb

  mov   rdi, [rsp]
  add   rdi, BCRYPT_BLOWFISH_OFF_S_BOXES
  mov   rsi, blowfish_const_s0
  mov   rcx, BCRYPT_S_BOXES_SIZE
  rep   movsb

  mov   rax, [rsp]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

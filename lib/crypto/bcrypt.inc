%include "blowfish_constants.inc"

section .bss

blowfish_state_t:
  .p_array  resb BCRYPT_P_ARRAY_SIZE
  .s_boxes  resb BCRYPT_S_BOXES_SIZE
blowfish_state_t_end:

section .data

test_salt db "9woyyttf8R4iXx1RveIKae", NULL_CHAR

BCRYPT_P_ARRAY_SIZE equ 72
BCRYPT_S_BOXES_SIZE equ 4096
BCRYPT_S_BOX_SIZE   equ 1024

BCRYPT_BLOWFISH_LEN equ blowfish_state_t_end - blowfish_state_t

BCRYPT_BLOWFISH_OFF_P_ARRAY equ 0x0
BCRYPT_BLOWFISH_OFF_S_BOXES equ BCRYPT_BLOWFISH_OFF_P_ARRAY + BCRYPT_P_ARRAY_SIZE

BCRYPT_OFF_VERSION  equ 0x0
BCRYPT_OFF_COST     equ BCRYPT_OFF_VERSION + 0x4
BCRYPT_OFF_SALT     equ BCRYPT_OFF_COST + 0x3 ; (cost + '$')
BCRYPT_OFF_HASH     equ BCRYPT_OFF_SALT + BCRYPT_SALT_SIZE

BCRYPT_DEFAULT_COST   equ 12
BCRYPT_MIN_COST       equ 4
BCRYPT_MAX_COST       equ 31

BCRYPT_SALT_SIZE          equ 22
BCRYPT_ENCODED_HASH_SIZE  equ 31
BCRYPT_HASH_SIZE          equ 61  ; include null char
BCRYPT_MAX_PWD_LEN        equ 73  ; include null char

bcrypt_magic_string db "OrpheanBeholderScryDoubt" ; no need for null char

bcrypt_base64_alphabet  db  "./ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                        db  "abcdefghijklmnopqrstuvwxyz"
                        db  "0123456789"

bcrypt_version db "$2b$", NULL_CHAR

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
; @param  rdi: pointer to the plain text password
; @return rax: SUCCESS if password is valid FAILURE otherwise
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
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> first generated number
  ; [rsp+0x8]   -> second generated number
  ; [rsp+0x10]  -> pointer to the salt

  ; generate first random 64-bit number
  call  rand_generate_number
  mov   [rsp], rax

  ; generate second random 64-bit number
  call  rand_generate_number
  mov   [rsp+0x8], rax

  mov   rdi, 16
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  mov   rdi, [rsp+0x10]
  lea   rsi, [rsp]
  mov   rcx, 16
  rep   movsb

  mov   rax, [rsp+0x10]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; runs encryption loop on the magic string
; the string is modified in-place
; @param  rdi: pointer to the blowfish state
; @param  rsi: pointer to the duplicated magic string
; @return rax: return code
_bcrypt_encrypt_loop:
  sub   rsp, 0x20

  ; STACK USAGE
  ; [rsp]       -> pointer to the blowfish state struct
  ; [rsp+0x8]   -> pointer to the duplicated magic string
  ; [rsp+0x10]  -> outer counter
  ; [rsp+0x18]  -> inner counte

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x10], 0
  mov   qword [rsp+0x18], 0

  cmp   rdi, 0
  je    .error

  cmp   rsi, 0
  je    .error

.outer_loop:
  mov   qword [rsp+0x18], 0

.inner_loop:
  mov   rcx, qword [rsp+0x10]
  mov   rax, [rsp+0x8]

  mov   rdi, [rsp]
  lea   rsi, [rax+rcx*8]
  call  _bcrypt_encrypt_block
  cmp   rax, 0
  jl    .error

  inc   qword [rsp+0x18]

  cmp   qword [rsp+0x18], 64
  jl    .inner_loop

.innner_loop_end:
  inc   qword [rsp+0x10]

  cmp   qword [rsp+0x10], 3
  jl    .outer_loop

.outer_loop_end:

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret

; encrypts in-place a 64-bit block using blowfish cypher
; @param  rdi: pointer to the blowfish state struct
; @param  rsi: pointer to the 64-bit block
; @param  rax: return code
_bcrypt_encrypt_block:
  sub   rsp, 0x10

  push  r12
  push  r13
  push  r14

  ; STACK USAGE
  ; [rsp]     -> pointer to the blowfish state struct
  ; [rsp+0x8] -> pointer to the 64-bit block

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  cmp   rdi, 0
  je    .error

  cmp   rsi, 0
  je    .error

  ; save L and R of the block in r13d and r14d
  mov   rdi, [rsp+0x8]
  mov   r14d, dword [rdi]       ; R
  mov   r13d, dword [rdi+0x4]   ; L

  xor   r12, r12

.loop:
  ; get P[i]
  mov   rdi, [rsp]
  mov   eax, dword [rdi+r12*4]

  ; L = L XOR P[i]
  xor   r13d, eax

  ; get f(L)
  mov   rdi, [rsp]
  mov   esi, r13d
  call  _bcrypt_blowfish_f_function
  cmp   rax, 0
  jl    .error

  ; R = f(L) XOR R
  xor   r14d, eax

  ; swap values of L and R
  mov   eax, r14d
  mov   r14d, r13d
  mov   r13d, eax

  inc   r12
  cmp   r12, 16
  jl    .loop

.loop_end:
  ; swap values of L and R
  mov   eax, r14d
  mov   r14d, r13d
  mov   r13d, eax

  ; get P[16]
  mov   rdi, [rsp]
  mov   eax, dword [rdi+16*4]

  ; R := R XOR P[16]
  xor   r14d, eax

  ; get P[17]
  mov   eax, dword [rdi+17*4]
  xor   r13d, eax

  ; update block in place
  mov   rdi, [rsp+0x8]
  mov   dword [rdi], r14d       ; R
  mov   dword [rdi+0x4], r13d   ; L

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  pop   r14
  pop   r13
  pop   r12

  add   rsp, 0x10
  ret

; calculates a function f on a 32-bit input
; @param  rdi: pointer to the blowfish state struct
; @param  rsi: 32-bit input
; @return rax: f(rsi)
_bcrypt_blowfish_f_function:
  push  rbx

  cmp   rdi, 0
  jle   .error

  ; S[0][high_byte]
  mov   edx, esi
  shr   edx, 24
  movzx ecx, dl
  mov   eax, [rdi+BCRYPT_BLOWFISH_OFF_S_BOXES+rcx*4]

  ; S[1][second_byte]
  mov   edx, esi
  shr   edx, 16
  movzx ecx, dl
  mov   ebx, [rdi+BCRYPT_BLOWFISH_OFF_S_BOXES+BCRYPT_S_BOX_SIZE+rcx*4]

  ; h := S[0][high_byte] + S[1][second_byte]
  add   eax, ebx

  ; S[2][third_byte]
  mov   edx, esi
  shr   edx, 8
  movzx ecx, dl
  mov   edx, [rdi+BCRYPT_BLOWFISH_OFF_S_BOXES+BCRYPT_S_BOX_SIZE*2+rcx*4]

  xor   eax, edx

  ; S[3][low_byte]
  movzx ecx, sil
  mov   edx, [rdi+BCRYPT_BLOWFISH_OFF_S_BOXES+BCRYPT_S_BOX_SIZE*3+rcx*4]

  ; return (h XOR S[2][third_byte]) + S[3][low_byte]
  add   eax, edx
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  pop   rbx
  ret

; concatenates the cost, the salt and the encrypted string
; user is responsible for freeing the final hash
; @param  rdi: cost
; @param  rsi: pointer to the salt
; @param  rdx: pointer to the encrypted magic string
; @return rax: pointer to the hash
_bcrypt_concat_values:
  sub   rsp, 0x30
  sub   rsp, TO_STRING_MAX_STR_SIZE

  ; STACK USAGE
  ; [rsp]       -> cost
  ; [rsp+0x8]   -> pointer to the salt
  ; [rsp+0x10]  -> pointer to the encrypted magic string
  ; [rsp+0x18]  -> pointer to the hash
  ; [rsp+0x20]  -> base64 encoded salt
  ; [rsp+0x28]  -> base64 encoded hash
  ; [rsp+0x30]  -> cost to string

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   qword [rsp+0x18], 0
  mov   qword [rsp+0x20], 0
  mov   qword [rsp+0x28], 0

  cmp   rdi, 0
  je    .error

  cmp   rsi, 0
  je    .error

  cmp   rdx, 0
  je    .error

  mov   rdi, BCRYPT_HASH_SIZE
  call  malloc
  cmp   rax, 0
  jle   .error

  mov   [rsp+0x18], rax

  ; copy version
  mov   rdi, [rsp+0x18]
  mov   rsi, bcrypt_version
  mov   rcx, 4
  rep   movsb

  ; to string the cost
  mov   rdi, [rsp]
  lea   rsi, [rsp+0x30]
  mov   rdx, TO_STRING_MAX_STR_SIZE
  call  itoa
  cmp   rax, 0
  jl    .error

  ; need to be padded if cost<10
  mov   rdi, [rsp+0x18]
  add   rdi, BCRYPT_OFF_COST
  mov   rcx, 2
  cmp   qword [rsp], 10
  jge   .no_padding

  mov   byte [rdi], ZERO_CHAR
  inc   rdi
  mov   rcx, 1

.no_padding:
  mov   rsi, rax
  rep   movsb

  ; add separator
  mov   rax, DOLLAR
  stosb

  ; base64 encode the salt
  mov   rdi, [rsp+0x8]
  mov   rsi, 16
  call  base64_encode
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  mov   rdi, [rsp+0x18]
  add   rdi, BCRYPT_OFF_SALT
  mov   rsi, [rsp+0x20]
  mov   rcx, BCRYPT_SALT_SIZE
  rep   movsb

  ; set the alphabet for next base64 encoding
  mov   rdi, bcrypt_base64_alphabet
  call  base64_set_alphabet
  cmp   rax, 0
  jl    .error

  ; disable padding
  call  base64_disable_padding
  cmp   rax, 0
  jl    .error

  ; base64 encode the hash
  mov   rdi, [rsp+0x10]
  mov   rsi, 23
  call  base64_encode
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x28], rax

  ; copy the hash
  mov   rdi, [rsp+0x18]
  add   rdi, BCRYPT_OFF_HASH
  mov   rsi, [rsp+0x28]
  mov   rcx, BCRYPT_ENCODED_HASH_SIZE
  rep   movsb

  mov   rax, NULL_CHAR
  stosb

  ; free the base64 encoded salt
  mov   rdi, [rsp+0x20]
  call  free
  mov   qword [rsp+0x20], 0
  cmp   rax, 0
  jl    .error

  ; free the base64 encoded hash
  mov   rdi, [rsp+0x28]
  call  free
  mov   qword [rsp+0x28], 0
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x18]

  jmp   .return

.error:
  mov   rdi, [rsp+0x18]
  test  rdi, rdi
  jz    .free_encoded_salt

  call  free

.free_encoded_salt:
  mov   rdi, [rsp+0x20]
  test  rdi, rdi
  jz    .free_encoded_hash

  call  free

.free_encoded_hash:
  mov   rdi, [rsp+0x28]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, TO_STRING_MAX_STR_SIZE
  add   rsp, 0x30
  ret

; initialises the blowfish state with cost, salt, and password
; @param  rdi: pointer to the password
; @param  rsi: pointer to the salt
; @param  rdx: cost
; @return rax: pointer to the state struct
_bcrypt_eks_blowfish_setup:
  sub   rsp, 0x30

  ; STACK USAGE
  ; [rsp]       -> pointer to the password
  ; [rsp+0x8]   -> pointer to the salt
  ; [rsp+0x10]  -> cost
  ; [rsp+0x18]  -> size of the password
  ; [rsp+0x20]  -> pointer to the state struct
  ; [rsp+0x28]  -> counter

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   qword [rsp+0x20], 0
  mov   qword [rsp+0x28], 0

  cmp   rdi, 0
  je    .error

  cmp   rsi, 0
  je    .error

  cmp   rdx, 0
  je    .error

  mov   rdi, [rsp]
  call  strlen
  cmp   rax, 0
  jl    .error

  inc   rax

  mov   [rsp+0x18], rax

  call  _bcrypt_init_state
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  ; permute P and S based on the password and salt
  mov   rdi, [rsp+0x20]
  mov   rsi, [rsp]
  mov   rdx, [rsp+0x8]
  mov   rcx, [rsp+0x18]
  call  _bcrypt_expand_key
  cmp   rax, 0
  jl    .error

  ; we need to get 2^cost which is equal to 1 << cost
  mov   rcx, [rsp+0x10]
  mov   rax, 1
  shl   rax, cl

  mov   qword [rsp+0x28], rax

.loop:
  mov   rdi, [rsp+0x20]
  mov   rsi, [rsp]
  mov   rdx, NO_ARG
  mov   rcx, [rsp+0x18]
  call  _bcrypt_expand_key
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x20]
  mov   rsi, [rsp+0x8]
  mov   rdx, NO_ARG
  mov   rcx, 16
  call  _bcrypt_expand_key
  cmp   rax, 0
  jl    .error

  dec   qword [rsp+0x28]
  cmp   qword [rsp+0x28], 0
  jle   .loop_end

  jmp   .loop
.loop_end:

  mov   rax, [rsp+0x20]
  jmp   .return

.error:
  mov   rdi, [rsp+0x20]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x30
  ret

; @param  rdi: pointer to the blowfish state struct
; @param  rsi: pointer to the password
; @param  rdx: pointer to the salt
; @param  rcx: length of the password
; @return rax: return code
_bcrypt_expand_key:
  sub   rsp, 0x30

  push  r9
  push  r10
  push  r11
  push  rbx

  ; STACK USAGE
  ; [rsp]       -> pointer to the blowfish state struct
  ; [rsp+0x8]   -> pointer to the password
  ; [rsp+0x10]  -> pointer to the decoded salt
  ; [rsp+0x18]  -> length of the password
  ; [rsp+0x20]  -> 64-bit block
  ; [rsp+0x28]  -> counter

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   [rsp+0x18], rcx
  mov   qword [rsp+0x20], 0
  mov   qword [rsp+0x28], 0

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  xor   r9, r9    ; counter
  xor   r10, r10  ; password index to store

.outer_loop:
  cmp   r9, BCRYPT_P_ARRAY_SIZE
  jge   .outer_loop_end

  xor   eax, eax
  xor   r11, r11  ; byte to dword

.inner_loop:
  cmp   r11, 4
  jge   .inner_loop_end

  shl   eax, 8

  ; get password[r10]
  mov   rdi, [rsp+0x8]
  movzx ebx, byte [rdi+r10]
  or    eax, ebx

  inc   r10
  inc   r11

  cmp   r10, [rsp+0x18]
  jl    .inner_loop

  xor   r10, r10

  jmp   .inner_loop

.inner_loop_end:
  ; get P[r9]
  mov   rdi, [rsp]
  xor   dword [rdi+r9*4], eax

  inc   r9

  jmp   .outer_loop

.outer_loop_end:

.encrypt_p_loop:
  cmp   qword [rsp+0x28], 8
  jg    .encrypt_p_loop_end

  ; block = block xor saltHalf[n mod 2]
  ; get the right salt half
  xor   rdx, rdx
  mov   rax, qword [rsp+0x28]
  and   rax, 1
  mov   rbx, 8
  mul   rbx

  xor   rdx, rdx
  mov   rdi, [rsp+0x10]
  test  rdi, rdi
  jz    .xor_salt_1

  mov   rdx, [rdi+rax]

.xor_salt_1:
  xor   [rsp+0x20], rdx

  mov   rdi, [rsp]
  lea   rsi, [rsp+0x20]
  call  _bcrypt_encrypt_block
  cmp   rax, 0
  jl    .error

  ; get P-array
  mov   rdx, [rsp]

  ; get the offset 8n
  mov   rax, [rsp+0x28]
  lea   rdi, [rdx + rax * 0x8]

  ; move block to p-array
  mov   rax, [rsp+0x20]
  ror   rax, 32
  mov   [rdi], rax

  add   qword [rsp+0x28], 0x1

  jmp   .encrypt_p_loop
.encrypt_p_loop_end:

  ; reset counter
  mov   qword [rsp+0x28], 0

.encrypt_s_loop:
  cmp   qword [rsp+0x28], 511
  jg    .encrypt_s_loop_end

  ; block = block xor saltHalf[n mod 2]
  ; get the right salt half
  xor   rdx, rdx
  mov   rax, qword [rsp+0x28]
  and   rax, 1
  mov   rbx, 8
  mul   rbx

  xor   rdx, rdx
  mov   rdi, [rsp+0x10]
  test  rdi, rdi
  jz    .xor_salt_2

  mov   rdx, [rdi+rax]

.xor_salt_2:
  xor   [rsp+0x20], rdx

  mov   rdi, [rsp]
  lea   rsi, [rsp+0x20]
  call  _bcrypt_encrypt_block
  cmp   rax, 0
  jl    .error

  ; get S-boxes
  mov   rdx, [rsp]

  ; get the offset 8n
  mov   rax, [rsp+0x28]
  lea   rdi, [rdx + rax * 0x8 + BCRYPT_BLOWFISH_OFF_S_BOXES]

  ; move block to s-array
  mov   rax, [rsp+0x20]
  ror   rax, 32
  mov   [rdi], rax

  add   qword [rsp+0x28], 0x1

  jmp   .encrypt_s_loop
.encrypt_s_loop_end:

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  pop   rbx
  pop   r11
  pop   r10
  pop   r9

  add   rsp, 0x30
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

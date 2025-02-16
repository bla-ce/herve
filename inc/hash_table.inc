; ******************************** WARNING ******************************* ;
;                                                                          ;
; This implementation of hash table only support string keys at the moment ;
;                                                                          ;
; ******************************** WARNING ******************************* ;

section .data

hash_table_struct:
  n_bucket  dq 0
  table     dq 0  ; table is an array of addresses pointing to an entry
hash_table_struct_end:

ht_entry_struct:
  ht_entry_key   dq 0  
  ht_entry_value dq 0
  ht_entry_next  dq 0
ht_entry_struct_end:

HT_STRUCT_LEN equ hash_table_struct_end - hash_table_struct

HT_OFF_NBUCKET  equ 0
HT_OFF_TABLE    equ HT_OFF_NBUCKET + 0x8

HT_ENTRY_STRUCT_LEN equ ht_entry_struct_end - ht_entry_struct

HT_ENTRY_OFF_KEY    equ 0
HT_ENTRY_OFF_VALUE  equ HT_ENTRY_OFF_KEY + 0x8
HT_ENTRY_OFF_NEXT   equ HT_ENTRY_OFF_VALUE + 0x8

section .text

; hash function to map values to key
; @param  rdi: pointer to hash
hash_string_key:
  sub   rsp, 0x10

  cmp   rdi, 0
  jle   .error

  mov   [rsp], rdi
  mov   qword [rsp+0x8], 0

.loop:
  xor   rax, rax
  mov   al, byte [rdi]
  movzx rax, al

  cmp   rax, 0
  je    .end_loop

  add   qword [rsp+0x8], rax
  inc   rdi

  jmp   .loop
.end_loop:

  mov   rax, qword [rsp+0x8]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; returns the index of the key given the hash
; @param  rdi: hash value
; @param  rsi: number of buckets
; @return rax: index of the key
get_index_from_hash:
  xor   rdx, rdx
  mov   rax, rdi
  div   rsi
  
  mov   rax, rdx

  ret

; returns the value of the associated key
; @param  rdi: pointer to the hash table
; @param  rsi: pointer to the key
; @return rax: pointer to the value
ht_get:
  sub   rsp, 0x20

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  ; hash string key
  mov   rdi, [rsp+0x8]
  call  hash_string_key

  cmp   rax, 0
  jl    .error

  ; get index from hash
  mov   rdi, rax

  mov   rax, [rsp]
  mov   rsi, qword [rax+HT_OFF_NBUCKET]
  call  get_index_from_hash

  ; store index
  mov   qword [rsp+0x10], rax 

  ; get table
  mov   rdi, [rsp]
  mov   rsi, [rdi+HT_OFF_TABLE]

  ; go to index
  xor   rdx, rdx
  mov   rax, qword [rsp+0x10]

  mov   rdi, [rsi+rax*8]

  ; if rdi is 0, this key does not exist
  cmp   rdi, 0
  je    .error

  ; save entry
  mov   [rsp+0x18], rdi

.loop:
  ; get key 
  mov   rsi, [rdi+HT_ENTRY_OFF_KEY]
  mov   rdi, [rsp+0x8]
  ; compare with wanted key
  call  cmpstr

  cmp   rax, TRUE
  ; if equal, jmp to the end of the loop
  je    .loop_end

  mov   rdi, [rsp+0x18]
  mov   rsi, [rdi+HT_ENTRY_OFF_NEXT]
  cmp   rsi, 0
  je    .error

  ; go to the next node on the linked list
  mov   rdi, rsi

  mov   [rsp+0x18], rdi

  jmp   .loop

.loop_end:
  mov   rdi, [rsp+0x18]

  ; get value from entry
  mov   rax, [rdi+HT_ENTRY_OFF_VALUE]

  jmp   .return  

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret


; inserts an entry into the hash table
; @param  rdi: pointer to the hash table
; @param  rsi: pointer to the key
; @param  rdx: pointer to the value
; @return rax: return value
ht_insert:
  sub   rsp, 0x40

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  cmp   rdx, 0
  jle   .error

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx

  ; create ht_entry
  ; malloc entry
  mov   rdi, HT_ENTRY_STRUCT_LEN
  call  malloc

  cmp   rax, 0
  jl    .error

  ; save entry
  mov   [rsp+0x18], rax

  ; store key
  mov   rdi, [rsp+0x8] 
  mov   [rax+HT_ENTRY_OFF_KEY], rdi

  ; store value
  mov   rdi, [rsp+0x10] 
  mov   [rax+HT_ENTRY_OFF_VALUE], rdi

  ; set next as 0
  mov   qword [rax+HT_ENTRY_OFF_NEXT], 0

  ; hash string
  mov   rdi, [rsp+0x8]
  call  hash_string_key

  cmp   rax, 0
  jl   .error

  mov   qword [rsp+0x20], rax

  ; get number of buckets
  mov   rax, [rsp]

  ; mod to the number of buckets
  mov   rdi, qword [rsp+0x20]
  mov   rsi, qword [rax+HT_OFF_NBUCKET]
  call  get_index_from_hash

  ; store index
  mov   qword [rsp+0x28], rax

  ; get hash table 
  mov   rsi, [rsp]
  mov   rdi, [rsi+HT_OFF_TABLE]

  ; check if there is an entry at the index
  mov   rbx, qword [rsp+0x28]

  xor   rdx, rdx
  mov   rax, rbx
  mov   rbx, 8
  mul   rbx
  add   rdi, rax

  ; rdi points to the position of the array
  mov   [rsp+0x38], rdi

  ; if key is 0, go to store key
  cmp   qword [rdi], 0
  je    .store_key

  ; check if current key is the same, if yes replace
  mov   rax, [rdi]

  mov   rdi, [rax+HT_ENTRY_OFF_KEY]
  mov   rsi, [rsp+0x8]
  call  cmpstr

  cmp   rax, 0
  jl    .error
  jg    .store_key  ; the key is the same, we can replace

  ; get array position
  mov   rdi, [rsp+0x38]

  ; get the entry
  mov   rax, [rdi]

  ; get the entry we want to add
  mov   rdi, [rsp+0x18]

  ; update next to point to the current entry
  mov   [rdi+HT_ENTRY_OFF_NEXT], rax

  ; now the entry we want to add is the head of the linked list and 
  ; next points to the previous head

.store_key:
  ; store ht entry 
  mov   rdi, [rsp+0x38]
  mov   rax, [rsp+0x18]
  mov   [rdi], rax
  
  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x40
  ret 
  
; returns a hash table of n buckets
; @param  rdi: number of buckets
; @return rax: pointer to the hash table
get_hash_table:
  sub   rsp, 0x10

  mov   [rsp], rdi

  ; malloc hash table struct
  mov   rdi, HT_STRUCT_LEN
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; alloc array of addresses pointing to entries
  mov   rdi, qword [rsp]   
  mov   rsi, 8  ; addresses of 8 bytes long
  call  calloc

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   qword [rdi+HT_OFF_TABLE], rax

  mov   rbx, qword [rsp]
  mov   qword [rdi+HT_OFF_NBUCKET], rbx
  
  mov   rax, [rsp+0x8]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret


; NOTE: this hash table implementation only supports strings as
; keys and values

%include "ht_entry.inc"

section .bss

hash_table_t:
  .ht_n_bucket  resq 1
  .ht_n_entries resq 1
  .ht_table     resq 1  ; table is an array of addresses pointing to an entry
hash_table_t_end:

section .data

HT_STRUCT_LEN   equ hash_table_t_end - hash_table_t

HT_OFF_NBUCKET  equ 0
HT_OFF_NENTRY   equ HT_OFF_NBUCKET + 0x8
HT_OFF_TABLE    equ HT_OFF_NENTRY + 0x8

section .text
; returns the number of entries in the hash table
; @param  rdi: pointer to the hash table
; @return rax: number of entries
ht_get_length:
  cmp   rdi, 0
  jle   .error

  mov   rax, qword [rdi+HT_OFF_NENTRY]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns an array struct containing the keys of the hash table
; user is responsible for freeing the array struct
; @param  rdi: pointer to hash table
; @return rax: pointer to array
ht_get_keys:
  push  r12

  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]       -> pointer to the hash table
  ; [rsp+0x8]   -> pointer to the array of keys
  ; [rsp+0x10]  -> temp entry

  mov   [rsp], rdi
  mov   qword [rsp+0x8], 0

  test  rdi, rdi
  jz    .error

  ; get the number of keys from the hash table
  mov   rdi, [rsp]
  call  ht_get_length
  cmp   rax, 0
  jl    .error
  je    .empty

  ; allocate an array of pointer
  mov   rdi, rax
  mov   rsi, 8
  call  array_create
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; get number of buckets
  mov   rdi, [rsp]
  call  ht_get_n_bucket
  cmp   rax, 0
  jl    .error

  ; counter
  mov   r12, rax
  dec   r12

.outer_loop:
  mov   rdi, [rsp]
  mov   rsi, r12
  call  ht_get_bucket_by_index
  cmp   rax, 0
  jl    .error

  mov   rdi, [rax]

  test  rdi, rdi
  je    .inner_loop_end

  mov   [rsp+0x10], rdi

.inner_loop:
  ; rdi is a pointer to a linked list struct
  mov   rdi, [rsp+0x10]
  call  linked_list_get_node_data
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rdi, [rsp+0x8]
  lea   rsi, [rax+HT_ENTRY_OFF_KEY]
  call  array_push
  cmp   rax, 0
  jl    .error

  ; go to next
  mov   rdi, [rsp+0x10]
  call  linked_list_get_node_next
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   [rsp+0x10], rax

  test  rax, rax
  jnz   .inner_loop

.inner_loop_end:
  dec   r12

  cmp   r12, 0
  jge   .outer_loop
.outer_loop_end:

  mov   rax, [rsp+0x8]

  jmp   .return

.empty:
  xor   rax, rax
  jmp   .return

.error:
  mov   rdi, [rsp+0x8]
  test  rdi, rdi
  jz    .no_free

  call  array_free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10

  pop   r12

  ret

; hash function to map values to key
; @param  rdi: pointer to hash
; @return rax: hash value
ht_hash_key:
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
  je    .loop_end

  add   qword [rsp+0x8], rax
  inc   rdi

  jmp   .loop
.loop_end:

  mov   rax, qword [rsp+0x8]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; returns the index of the key given the hash
; @param  rdi: pointer to the hash table struct
; @param  rsi: hash
; @return rax: index of the key
ht_get_bucket_idx_from_hash:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> pointer to the hash table struct
  ; [rsp+0x8] -> hash

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  ; get number of buckets
  mov   rdi, [rsp]
  call  ht_get_n_bucket
  cmp   rax, 0
  jl    .error

  xor   rdx, rdx
  mov   rbx, rax
  mov   rax, [rsp+0x8]
  div   rbx

  mov   rax, rdx

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; returns the value of the associated key
; @param  rdi: pointer to the hash table
; @param  rsi: pointer to the key
; @return rax: pointer to the value or 0 if it does not exist
ht_get:
  sub   rsp, 0x20

  ; STACK USAGE
  ; [rsp]       -> pointer to the hash table
  ; [rsp+0x8]   -> pointer to the key
  ; [rsp+0x10]  -> hash value of the key
  ; [rsp+0x18]  -> temp entry

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  ; hash string key
  mov   rdi, [rsp+0x8]
  call  ht_hash_key
  cmp   rax, 0
  jl    .error

  ; get index from hash
  mov   rdi, [rsp]
  mov   rsi, rax
  call  ht_get_bucket_idx_from_hash
  cmp   rax, 0
  jl    .error

  ; store index
  mov   qword [rsp+0x10], rax

  ; get table
  mov   rdi, [rsp]
  mov   rsi, [rdi+HT_OFF_TABLE]

  ; go to index
  mov   rdi, [rsi+rax*8]

  ; if rdi is 0, this key does not exist
  test  rdi, rdi
  jz    .does_not_exist

  ; save entry
  mov   [rsp+0x18], rdi

.loop:
  mov   rdi, [rsp+0x18]
  call  linked_list_get_node_data
  cmp   byte [linked_list_errno], TRUE
  je    .error

  ; get key
  mov   rsi, [rax+HT_ENTRY_OFF_KEY]
  mov   rdi, [rsp+0x8]
  ; compare with wanted key
  call  strcmp
  cmp   rax, TRUE
  ; if equal, jmp to the end of the loop
  je    .loop_end

  mov   rdi, [rsp+0x18]
  call  linked_list_get_node_next
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   [rsp+0x18], rax

  test  rax, rax
  jnz   .loop

  jmp   .does_not_exist

.loop_end:
  mov   rdi, [rsp+0x18]
  call  linked_list_get_node_data
  cmp   byte [linked_list_errno], TRUE
  je    .error

  ; get value from entry
  mov   rax, [rax+HT_ENTRY_OFF_VALUE]

  jmp   .return

.does_not_exist:
  xor   rax, rax
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret

; deletes an entry from the hash table
; @param  rdi: pointer to the hash table
; @param  rsi: pointer to the key
; @param  rax: return value
ht_del:
  sub   rsp, 0x20

  ; STACK USAGE
  ; [rsp]       -> pointer to the hash table
  ; [rsp+0x8]   -> pointer to the key
  ; [rsp+0x10]  -> pointer to the bucket
  ; [rsp+0x18]  -> pointer to the current entry

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  ; get index from string
  mov   rdi, [rsp+0x8]
  call  ht_hash_key
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, rax
  call  ht_get_bucket_idx_from_hash
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, rax
  call  ht_get_bucket_by_index
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  mov   rdi, [rax]

  ; check bucket empty
  test  rdi, rdi
  jz    .error

  mov   [rsp+0x18], rdi

.loop:
  mov   rdi, [rsp+0x18]
  call  linked_list_get_node_data
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rdi, rax
  call  ht_entry_get_key
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, [rsp+0x8]
  call  strcmp
  cmp   rax, 0
  jl    .error

  cmp   rax, TRUE
  je    .loop_end

  mov   rdi, [rsp+0x18]
  call  linked_list_get_node_next
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   [rsp+0x18], rax

  test  rax, rax
  jnz   .loop

  ; not found
  jmp   .does_not_exist

.loop_end:
  mov   rdi, [rsp+0x18]
  call  linked_list_get_node_data
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rdi, [rsp+0x10]
  mov   rsi, rax
  mov   rdx, ht_entry_free
  call  linked_list_delete_from_value
  cmp   byte [linked_list_errno], TRUE
  je    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.does_not_exist:
  xor   rax, rax
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret

; returns the address of a bucket by its index
; @param  rdi: pointer to the hash table struct
; @param  rsi: index
; @return rax: pointer to the bucket
ht_get_bucket_by_index:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> pointer to the ht struct
  ; [rsp+0x8] -> index of the bucket

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  ; get number of bucket
  mov   rdi, [rsp]
  call  ht_get_n_bucket
  cmp   rax, 0
  jl    .error

  ; invalid index
  cmp   [rsp+0x8], rax
  jge   .error

  mov   rdi, [rsp]
  call  ht_get_table
  cmp   rax, 0
  jl    .error

  mov   rdi, rax

  xor   rdx, rdx
  mov   rax, [rsp+0x8]
  mov   rbx, 8
  mul   rbx

  add   rdi, rax

  mov   rax, rdi

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; inserts an entry into the hash table
; @param  rdi: pointer to the hash table
; @param  rsi: pointer to the key
; @param  rdx: value
; @return rax: return value
ht_insert:
  sub   rsp, 0x38

  ; STACK USAGE
  ; [rsp]       -> pointer to the hash table
  ; [rsp+0x8]   -> pointer to the key
  ; [rsp+0x10]  -> value
  ; [rsp+0x18]  -> pointer to the new entry
  ; [rsp+0x20]  -> hash value of the key
  ; [rsp+0x28]  -> index of the entry
  ; [rsp+0x30]  -> pointer to the array of entry at the index

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   qword [rsp+0x18], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; check key length
  mov   rdi, [rsp+0x8]
  call  strlen
  cmp   rax, 0
  jl    .error

  cmp   rax, HT_ENTRY_KEY_MAX_LENGTH
  jg    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x10]
  call  ht_entry_create
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; check if the key already exist
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  call  ht_get
  cmp   rax, 0
  je    .no_delete

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  call  ht_del
  cmp   rax, 0
  jl    .error

.no_delete:
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x18]
  call  ht_add_entry
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rdi, [rsp+0x18]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x38
  ret

; returns a hash table of n buckets
; @param  rdi: number of buckets
; @return rax: pointer to the hash table
ht_create:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> number of buckets
  ; [rsp+0x8] -> pointer to the hash table

  mov   [rsp], rdi
  mov   qword [rsp+0x8], 0

  cmp   rdi,  0
  jl    .error
  je    .ht_empty

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
  je    .ht_empty

  mov   rdi, [rsp+0x8]
  mov   qword [rdi+HT_OFF_TABLE], rax

  mov   rbx, qword [rsp]
  mov   qword [rdi+HT_OFF_NBUCKET], rbx

  mov   qword [rdi+HT_OFF_NENTRY], 0

  mov   rax, [rsp+0x8]
  jmp   .return

.ht_empty:
  xor   rax, rax
  jmp   .return

.error:
  mov   rdi, [rsp+0x8]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; frees the hash table and allocated resources
; @param  rdi: pointer to the hash table
; @return rax: return code
ht_free:
  sub   rsp, 0x20

  ; STACK USAGE
  ; [rsp]       -> pointer to the hash table
  ; [rsp+0x8]   -> number of buckets
  ; [rsp+0x10]  -> pointer to the table
  ; [rsp+0x18]  -> counter

  mov   [rsp], rdi
  mov   qword [rsp+0x18], 0

  cmp   rdi, 0
  jle   .error

  ; get the number of buckets in the hash table
  mov   rdi, [rsp]
  call  ht_get_n_bucket
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, [rsp]
  call  ht_get_table
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

.loop:
  ; get table
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x18]
  call  ht_get_bucket_by_index
  cmp   rax, 0
  jl    .error

  ; go to bucket address
  mov   rdi, rax
  call  ht_bucket_free
  cmp   rax, 0
  jl    .error

  inc   qword [rsp+0x18]

  mov   rax, [rsp+0x8]
  cmp   qword [rsp+0x18], rax
  jge   .loop_end

  jmp   .loop

.loop_end:
  ; free hash table
  mov   rdi, [rsp+0x10]
  call  free
  cmp   rax, 0
  jl    .error

  ; free hash table struct
  mov   rdi, [rsp]
  call  free
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret

; frees all the entries inside a specific ht bucket
; @param  rdi: address of the bucket
; @return rax: return code
ht_bucket_free:
  test  rdi, rdi
  jz    .error

  mov   rsi, ht_entry_free
  call  linked_list_free

.done:
  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; adds a ht entry to the hash table
; @param  rdi: pointer to the hash table
; @param  rsi: pointer to the entry
; @return rax: return code
ht_add_entry:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> pointer to the hash table struct
  ; [rsp+0x8]   -> pointer to the entry
  ; [rsp+0x10]  -> key of the entry

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; get the key of the entry
  mov   rdi, [rsp+0x8]
  call  ht_entry_get_key
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; hash string
  mov   rdi, [rsp+0x10]
  call  ht_hash_key
  cmp   rax, 0
  jl   .error

  ; get number of bucket
  mov   rdi, [rsp]
  mov   rsi, rax
  call  ht_get_bucket_idx_from_hash
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, rax
  call  ht_get_bucket_by_index
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, [rsp+0x8]
  call  linked_list_insert_at_first
  cmp   byte [linked_list_errno], TRUE
  je    .error

  ; increase the number of entry
  mov   rdi, [rsp]
  inc   qword [rdi+HT_OFF_NENTRY]

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; returns the number of buckets in the hash table
; @param  rdi: pointer to the ht struct
; @return rax: number of buckets
ht_get_n_bucket:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+HT_OFF_NBUCKET]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the actual hash table from the struct
; @param  rdi: pointer to the ht struct
; @return rax: pointer to the ht element
ht_get_table:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+HT_OFF_TABLE]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

section .bss
  fd_set_struct:
    fds_bits  resd bits_len
  fd_set_struct_end:

section .data
  FD_SETSIZE equ 1024

  MAX_CONNECT equ 20

  bits_len equ FD_SETSIZE / 8 / UNSIGNED_LONG_MAX_STR_SIZE

  FD_SET_STRUCT_LEN equ fd_set_struct_end - fd_set_struct

section .text

; remove a file descriptor from the set
; @param  rdi: file descriptor to remove
; @param  rsi: pointer to file descriptors set
; @return rax: return code
FD_CLR:
  sub   rsp, 0x28

  cmp   rdi, 0
  jl    .error

  cmp   rsi, 0
  jle   .error

  ; *** STACK USAGE *** ;
  ; [rsp]       -> file descriptor to remove
  ; [rsp+0x8]   -> pointer to file descriptors set
  ; [rsp+0x10]  -> 8 * sizeof(long)
  ; [rsp+0x18]  -> index that needs to be updated
  ; [rsp+0x20]  -> shifted value

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  ; get index
  xor   rdx, rdx
  mov   rax, 8
  mov   rbx, UNSIGNED_LONG_MAX_STR_SIZE
  mul   rbx

  mov   [rsp+0x10], rax

  xor   rdx, rdx
  mov   rax, [rsp]
  div   rbx

  mov   [rsp+0x18], rax

  ; get unshifted value
  xor   rdx, rdx
  mov   rax, [rsp]
  mov   rbx, [rsp+0x10]
  div   rbx

  ; remainder in rdx
  mov   rcx, rdx
  mov   rax, 1

  ; shift value
  shl   rax, cl

  not   rax

  mov   [rsp+0x20], rax

  ; get prev index value
  xor   rdx, rdx
  mov   rax, [rsp+0x18]
  mov   rbx, UNSIGNED_LONG_MAX_STR_SIZE
  mul   rbx

  mov   rdi, [rsp+0x8]
  add   rdi, rax

  ; and value with prev
  mov   rsi, [rdi]
  and   rsi, [rsp+0x20]
  mov   [rdi], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x28
  ret

; removes all file descriptors from the set
; @param  rdi: pointer to file descriptors set
; @return rax: return code
FD_ZERO:
  cmp   rdi, 0
  jl    .error

  mov   rsi, 0
  mov   rdx, FD_SET_STRUCT_LEN
  call  memset
  
  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; adds a file descriptor to the set
; @param  rdi: file descriptor to add
; @param  rsi: pointer to file descriptors set
; @param  rax: return code
FD_SET:
  sub   rsp, 0x28

  cmp   rdi, 0
  jl    .error

  cmp   rsi, 0
  jle   .error

  ; *** STACK USAGE *** ;
  ; [rsp]       -> file descriptor to add
  ; [rsp+0x8]   -> pointer to file descriptors set
  ; [rsp+0x10]  -> 8 * sizeof(long)
  ; [rsp+0x18]  -> index that needs to be updated
  ; [rsp+0x20]  -> shifted value

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  ; get index
  xor   rdx, rdx
  mov   rax, 8
  mov   rbx, UNSIGNED_LONG_MAX_STR_SIZE
  mul   rbx

  mov   [rsp+0x10], rax

  xor   rdx, rdx
  mov   rax, [rsp]
  div   rbx

  mov   [rsp+0x18], rax

  ; get unshifted value
  xor   rdx, rdx
  mov   rax, [rsp]
  mov   rbx, [rsp+0x10]
  div   rbx

  ; remainder in rdx
  mov   rcx, rdx
  mov   rax, 1

  ; shift value
  shl   rax, cl

  mov   [rsp+0x20], rax

  ; get prev index value
  xor   rdx, rdx
  mov   rax, [rsp+0x18]
  mov   rbx, UNSIGNED_LONG_MAX_STR_SIZE
  mul   rbx

  mov   rdi, [rsp+0x8]
  add   rdi, rax

  ; or value with prev
  mov   rsi, [rdi]
  or    rsi, [rsp+0x20]
  mov   [rdi], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x28
  ret

; returns if a file descriptor is still present in the set
; @param  rdi: file descriptor to add
; @param  rsi: pointer to file descriptors set
; @param  rax: return code
FD_ISSET:
  sub   rsp, 0x28

  cmp   rdi, 0
  jl    .error

  cmp   rsi, 0
  jle   .error

  ; *** STACK USAGE *** ;
  ; [rsp]       -> file descriptor to add
  ; [rsp+0x8]   -> pointer to file descriptors set
  ; [rsp+0x10]  -> 8 * sizeof(long)
  ; [rsp+0x18]  -> index that needs to be updated
  ; [rsp+0x20]  -> shifted value

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  ; get index
  xor   rdx, rdx
  mov   rax, 8
  mov   rbx, UNSIGNED_LONG_MAX_STR_SIZE
  mul   rbx

  mov   [rsp+0x10], rax

  xor   rdx, rdx
  mov   rax, [rsp]
  div   rbx

  mov   [rsp+0x18], rax

  ; get unshifted value
  xor   rdx, rdx
  mov   rax, [rsp]
  mov   rbx, [rsp+0x10]
  div   rbx

  ; remainder in rdx
  mov   rcx, rdx
  mov   rax, 1

  ; shift value
  shl   rax, cl

  mov   [rsp+0x20], rax

  ; get prev index value
  xor   rdx, rdx
  mov   rax, [rsp+0x18]
  mov   rbx, UNSIGNED_LONG_MAX_STR_SIZE
  mul   rbx

  mov   rdi, [rsp+0x8]
  add   rdi, rax

  ; or value with prev
  mov   rax, [rdi]
  and   rax, [rsp+0x20]

  not   rax
  not   rax

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x28
  ret

; removes the client from the clients array
; @param  rdi: file descriptor of the client
; @param  rsi: current number of active connections
; @return rax: return code
remove_client_from_array:
  sub   rsp, 0x10

  ; *** STACK USAGE *** ;
  ; [rsp]     -> file descriptor
  ; [rsp+0x8] -> current number of active connections

  mov   [rsp], rdi 
  mov   [rsp+0x8], rsi

  cmp   rdi, 0
  jl    .error

  xor   rax, rax        ; index
  mov   rbx, rdi        ; store fd in rbx
  mov   rsi, clients

.loop:
  mov   rdi, [rsi] 
  cmp   rdi, 0
  je    .null_struct

  cmp   qword [rdi+CLIENT_OFF_CLIENT_FD], rbx
  je    .loop_end

.null_struct:
  ; move to next struct
  add   rsi, 8

  inc   rax
  cmp   rax, qword [rsp+0x8]
  jg    .does_not_exist

  jmp   .loop

.loop_end:
  mov   qword [rsi], 0

  jmp   .return

.does_not_exist:
  mov   rax, 0
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; returns the client struct with the file descriptor 
; or return 0 if does not exist
; @param  rdi: file descriptor
; @param  rsi: current number of active connections
; @return rax: pointer to the user struct or null
get_client_by_fd:
  sub   rsp, 0x10

  ; *** STACK USAGE *** ;
  ; [rsp]     -> file descriptor
  ; [rsp+0x8] -> current number of active connections

  mov   [rsp], rdi 
  mov   [rsp+0x8], rsi

  cmp   rdi, 0
  jl    .error

  xor   rax, rax        ; index
  mov   rbx, rdi        ; store fd in rbx
  mov   rsi, clients

.loop:
  mov   rdi, [rsi] 
  cmp   rdi, 0
  je    .null_struct

  cmp   qword [rdi+CLIENT_OFF_CLIENT_FD], rbx
  je    .loop_end

.null_struct:
  ; move to next struct
  add   rsi, 8

  inc   rax
  cmp   rax, qword [rsp+0x8]
  jg    .does_not_exist

  jmp   .loop

.loop_end:

  mov   rax, rdi
  jmp   .return

.does_not_exist:
  mov   rax, 0
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret


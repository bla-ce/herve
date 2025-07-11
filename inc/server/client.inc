section .bss
client_struct:
  client_fd       resq 1
  client_sockaddr resq 1
client_struct_end:

section .data

  CLIENT_STRUCT_LEN equ client_struct_end - client_struct

  CLIENT_OFF_CLIENT_FD equ 0x0
  CLIENT_OFF_SOCKADDR  equ CLIENT_OFF_CLIENT_FD + 0x8

  IPV4_MAX_LENGTH equ 0x10

section .text
; logs the client ipv4
; @param  rdi: pointer to the logger struct
; @param  rsi: pointer to the context struct
log_client_ip:
  sub   rsp, 0x10
  sub   rsp, IPV4_MAX_LENGTH

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the logger struct
  ; [rsp+0x8]   -> pointer to the context struct 
  ; [rsp+0x10]  -. pointer to the ip string

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x10], 0

  ; get client struct
  mov   rdi, [rsp+0x8]
  call  get_ctx_client
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  lea   rsi, [rsp+0x10]
  call  get_client_ip
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  lea   rsi, [rsp+0x10]
  call  log
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, IPV4_MAX_LENGTH
  add   rsp, 0x10
  ret

; sets the file descriptor of the client
; @param  rdi: pointer to the client
; @param  rsi: file descriptor
; @return rax: return code
set_client_fd:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   qword [rdi+CLIENT_OFF_CLIENT_FD], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the file descriptor of the client
; @param  rdi: pointer to the client
; @return rax: file descriptor
get_client_fd:
  cmp   rdi, 0
  jle   .error

  mov   rax, qword [rdi+CLIENT_OFF_CLIENT_FD]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret


; returns the string representation of the client ipv4 address
; developer is responsible of freeing the string
; @param  rdi: pointer to the client struct
; @return rax: pointer to the string 
get_client_ip:
  sub   rsp, 0x38
  sub   rsp, SOCKADDR_STRUCT_LEN

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the client struct
  ; [rsp+0x8]   -> client fd
  ; [rsp+0x10]  -> pointer to the string representation of the ip
  ; [rsp+0x18]  -> dynamic pointer for the string
  ; [rsp+0x20]  -> dynamic pointer sockaddr_addrin
  ; [rsp+0x28]  -> temp string to store to convert int to string
  ; [rsp+0x38]  -> sockaddr struct

  mov   [rsp], rdi
  mov   [rsp+0x10], rsi
  mov   [rsp+0x18], rsi

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error
  
  ; get client fd
  mov   rdi, [rsp]
  call  get_client_fd
  cmp   rax, 0
  jl    .error

  mov   qword [rsp+0x8], rax

  ; getpeername
  mov   rax, SYS_GETPEERNAME
  mov   rdi, qword [rsp+0x8]
  lea   rsi, [rsp+0x38]
  lea   rdx, [addrlen]
  syscall
   
  cmp   rax, 0
  jl    .error

  ; get addr_in
  lea   rax, [rsp+0x38]
  add   rax, SOCKADDR_OFF_SIN_ADDR
  mov   [rsp+0x20], rax

  mov   qword [rsp+0x30], 3 ; three dots for IPV4

.loop:
  mov   rax, [rsp+0x20]
  mov   dil, byte [rax]
  movzx rdi, dil
  inc   rax
  mov   [rsp+0x20], rax

  cmp   qword [rsp+0x30], 0
  jl   .end_loop

  lea   rsi, [rsp+0x28]
  mov   rdx, 4  ; add one for null char
  call  to_string
  cmp   rax, 0
  jl    .error

  ; copy
  mov   rdi, [rsp+0x18]
  mov   rsi, rax
  call  strcpy
  cmp   rax, 0
  jl    .error

  ; add dot
  mov   rdi, rax
  mov   rax, DOT
  stosb

  mov   [rsp+0x18], rdi

  dec   qword [rsp+0x30]

  jmp   .loop

.end_loop:
  ; ip has an extra dot, go back one and add null char
  mov   rdi, [rsp+0x18]
  dec   rdi
  mov   rax, NULL_CHAR
  stosb

  mov   rax, [rsp+0x10]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, SOCKADDR_STRUCT_LEN
  add   rsp, 0x38
  ret


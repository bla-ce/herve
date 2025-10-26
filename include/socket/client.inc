section .bss
client_struct:
  client_fd       resq 1
  client_sockaddr resq 1
  client_ip       resq 1
client_struct_end:

section .data

  CLIENT_STRUCT_LEN equ client_struct_end - client_struct

  CLIENT_OFF_CLIENT_FD  equ 0x0
  CLIENT_OFF_SOCKADDR   equ CLIENT_OFF_CLIENT_FD + 0x8
  CLIENT_OFF_IP         equ CLIENT_OFF_SOCKADDR + 0x8

  IPV4_MAX_LENGTH equ 0x10

section .text
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

; sets the ip of the client struct
; @param  rdi: pointer to the client struct
; @param  rsi: pointer to the ip
; @return rax: return code
set_client_ip:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   [rdi+CLIENT_OFF_IP], rsi

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the ip from the client struct
; @param  rdi: pointer to the client struct
; @return rax: pointer to the ip
get_client_ip:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+CLIENT_OFF_IP]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the string representation of the client ipv4 address
; @param  rdi: pointer to the client struct
; @param  rsi: pointer to the string
; @return rax: return code
ip_to_str:
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

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, SOCKADDR_STRUCT_LEN
  add   rsp, 0x38
  ret

; free the client struct and associated resources
; @param  rdi: pointer to the client struct
; @return rax: return code
free_client:
  sub   rsp, 0x8

  ; *** STACK USAGE *** ;
  ; [rsp] -> pointer to the client struct

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

  ; free the client ip string
  mov   rdi, [rsp]
  call  get_client_ip
  cmp   rax, 0
  jle   .free_struct

  mov   rdi, rax
  call  free
  cmp   rax, 0
  jl    .error

.free_struct:
  ; free the client
  mov   rdi, [rsp]
  call  free
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; close the connection and clear fd from the set
; @param  rdi: pointer to the file descriptor of the client
; @param  rsi: pointer to the main FD_SET
; @param  rdx: current number of connections
; @return rax: return code
clear_client_connection:
  sub   rsp, 0x18

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the fd of the client
  ; [rsp+0x8]   -> pointer to the main FD_SET
  ; [rsp+0x10]  -> current number of connections

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx

  cmp   rdi, 0
  jl    .error

  cmp   rsi, 0
  jle   .error

  cmp   rdx, 0
  jle   .error

  ; close connection
  mov   rdi, qword [rsp]
  call  close_file

  ; clear the file descriptor from the set
  mov   rdi, qword [rsp]
  mov   rsi, [rsp+0x8]
  call  FD_CLR
  cmp   rax, 0
  jl    .error

  mov   rdi, qword [rsp]
  mov   rsi, qword [rsp+0x10]
  call  get_client_by_fd
  cmp   rax, 0
  jle   .error

  ; free client struct
  mov   rdi, rax
  call  free_client
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret


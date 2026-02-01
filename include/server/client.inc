section .bss
client_t:
  .fd       resq 1
  .ip       resq 1
client_t_end:

section .data

CLIENT_STRUCT_LEN equ client_t_end - client_t

CLIENT_OFF_FD  equ 0x0
CLIENT_OFF_IP         equ CLIENT_OFF_FD + 0x8

IPV4_MAX_LENGTH equ 0x10

section .text
; sets the file descriptor of the client
; @param  rdi: pointer to the client
; @param  rsi: file descriptor
; @return rax: return code
client_set_fd:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   qword [rdi+CLIENT_OFF_FD], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the file descriptor of the client
; @param  rdi: pointer to the client
; @return rax: file descriptor
client_get_fd:
  cmp   rdi, 0
  jle   .error

  mov   rax, qword [rdi+CLIENT_OFF_FD]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the ip of the client struct
; @param  rdi: pointer to the client struct
; @param  rsi: pointer to the ip
; @return rax: return code
client_set_ip:
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
client_get_ip:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+CLIENT_OFF_IP]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the string representation of the client ipv4 address
; @param  rdi: file descriptor of the client
; @param  rsi: pointer to the string
; @return rax: return code
client_get_ip_from_fd:
  sub   rsp, 0x30
  sub   rsp, SOCKADDR_STRUCT_LEN

  ; STACK USAGE
  ; [rsp]       -> client fd
  ; [rsp+0x8]   -> pointer to the string representation of the ip
  ; [rsp+0x10]  -> dynamic pointer for the string
  ; [rsp+0x18]  -> dynamic pointer sockaddr_addrin
  ; [rsp+0x20]  -> tmp str to store int to str (8 bytes is enough)
  ; [rsp+0x28]  -> counter
  ; [rsp+0x30]  -> sockaddr struct

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rsi

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; getpeername
  mov   rax, SYS_GETPEERNAME
  mov   rdi, [rsp]
  lea   rsi, [rsp+0x30]
  lea   rdx, [addrlen]
  syscall
  cmp   rax, 0
  jl    .error

  ; get addr_in
  lea   rax, [rsp+0x30]
  add   rax, SOCKADDR_OFF_SIN_ADDR
  mov   [rsp+0x18], rax

  mov   qword [rsp+0x28], 3 ; three dots for IPV4

.loop:
  mov   rax, [rsp+0x18]
  mov   dil, byte [rax]
  movzx rdi, dil
  inc   rax
  mov   [rsp+0x18], rax

  cmp   qword [rsp+0x28], 0
  jl   .end_loop

  lea   rsi, [rsp+0x20]
  mov   rdx, 4  ; add one for null char
  call  itoa
  cmp   rax, 0
  jl    .error

  ; copy
  mov   rdi, [rsp+0x10]
  mov   rsi, rax
  call  strcpy
  cmp   rax, 0
  jl    .error

  ; add dot
  mov   rdi, rax
  mov   rax, DOT
  stosb

  mov   [rsp+0x10], rdi

  dec   qword [rsp+0x28]

  jmp   .loop

.end_loop:
  ; ip has an extra dot, go back one and add null char
  mov   rdi, [rsp+0x10]
  dec   rdi
  mov   rax, NULL_CHAR
  stosb

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, SOCKADDR_STRUCT_LEN
  add   rsp, 0x30
  ret

; free the client struct and associated resources
; @param  rdi: pointer to the client struct
; @return rax: return code
client_free:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp] -> pointer to the client struct

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

  ; free the client ip string
  mov   rdi, [rsp]
  call  client_get_ip
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

; initialises a new client struct
; @param  rdi: file descriptor of the client
; @return rax: pointer to the client struct
client_init:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> file descriptor of the client
  ; [rsp+0x8]   -> pointer to the client struct
  ; [rsp+0x10]  -> pointer to the str ip

  mov   [rsp], rdi
  mov   qword [rsp+0x8], 0
  mov   qword [rsp+0x10], 0

  cmp   rdi, 0
  jl    .error

  ; malloc client struct
  mov   rdi, CLIENT_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; populate struct with fd and empty id
  mov   rbx, [rsp]
  mov   [rax+CLIENT_OFF_FD], rbx
  mov   qword [rax+CLIENT_OFF_IP], 0

  ; malloc string for the ip
  mov   rdi, IPV4_MAX_LENGTH
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; from the fd, get the client ip
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x10]
  call  client_get_ip_from_fd
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x10]
  call  client_set_ip
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x8]

  jmp   .return

.error:
  mov   rdi, [rsp+0x8]
  test  rdi, rdi
  jz    .free_ip_str

  call  free

.free_ip_str:
  mov   rdi, [rsp+0x10]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

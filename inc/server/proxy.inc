section .data
proxy_struct:
  _proxy_url   dq 0
  _proxy_port  dq 0 ; TODO: word is enough here
proxy_struct_end:

PROXY_STRUCT_LEN equ proxy_struct_end - proxy_struct

PROXY_OFF_URL   equ 0x0
PROXY_OFF_PORT  equ PROXY_OFF_URL + 0x8

section .text
; configures a proxy middleware
; @param  rdi: pointer to the context struct
; @param  rsi: pointer to the proxy struct
; @return rax: success code
proxy_middleware:
  sub   rsp, 0x30

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  ; connect to client   
  mov   rdi, [rsp+0x8]
  call  proxy_configure_server
  cmp   rax, 0
  jl    .error

  ; save file descriptor
  mov   qword [rsp+0x10], rax

  ; sends the request
  mov   rdi, [rsp]
  call  get_ctx_request
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  get_request_str

  mov   [rsp+0x18], rax

  mov   rdi, rax
  call  strlen
  cmp   rax, 0
  jl    .error

  ; sends request
  mov   rdi, qword [rsp+0x10]
  mov   rsi, [rsp+0x18]
  mov   rdx, rax
  call  write_file
  cmp   rax, 0
  jl    .error

  mov   rdi, RESPONSE_MAX_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  ; receive response
  ; TODO: in its own function
  mov   rsi, rax
  mov   rax, SYS_RECVFROM
  mov   rdi, qword [rsp+0x10]
  mov   rdx, RESPONSE_MAX_LEN
  xor   r10, r10
  xor   r9, r9
  xor   r8, r8
  syscall

  cmp   rax, 0
  jl    .error
  
  mov   qword [rsp+0x28], rax

  ; add null char at the end of the request
  mov   rdi, [rsp+0x20]
  add   rdi, rax
  xor   rax, rax
  stosb

  ; get client fd
  mov   rdi, [rsp]
  call  get_ctx_client
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  get_client_fd
  cmp   rax, 0
  jl    .error

  ; send back the response to the client
  ; TODO: in its own function
  mov   rdi, rax
  mov   rsi, [rsp+0x20]
  mov   rdx, qword [rsp+0x28]
  call  write_file
  cmp   rax, 0
  jl    .error

  ; close connection with end server TODO: in its own func
  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp+0x10]
  syscall

  ; abort request
  ; TODO: is it really how it should be done?
  mov   rdi, [rsp]
  call  ctx_abort

  ; TODO: parse response for logging status code

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x30
  ret

; configures the socket for the end server
; @param  rdi: pointer to the proxy struct
; @return rax: file descriptor
proxy_configure_server:
  sub   rsp, 0x10
  sub   rsp, SOCKADDR_STRUCT_LEN

  cmp   rdi, 0
  jle   .error

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the proxy struct
  ; [rsp+0x8]   -> file descriptor
  ; [rsp+0x10]  -> pointer to request string
  ; [rsp+0x18]  -> sockaddr struct

  mov   [rsp], rdi

  ; create socket
  mov   rdi, AF_INET
  mov   rsi, SOCK_STREAM
  mov   rdx, 0
  call  create_socket
  cmp   rax, 0
  jl    .error

  mov   qword [rsp+0x8], rax

  ; set socket options
  mov   rdi, qword [rsp+0x8]
  mov   rsi, SOL_SOCKET
  mov   rdx, SO_REUSEPORT
  call  set_socket_option

  cmp   rax, 0
  jl    .error

  mov   rdi, qword [rsp+0x8]
  mov   rsi, SOL_SOCKET
  mov   rdx, SO_REUSEADDR
  call  set_socket_option

  cmp   rax, 0
  jl    .error

  ; convert port to network port
  mov   rsi, [rsp]
  mov   rdi, qword [rsi+PROXY_OFF_PORT]
  call  htons

  lea   rdi, [rsp+0x10]
  mov   word [rdi+SOCKADDR_OFF_SIN_PORT], ax

  mov   word [rdi+SOCKADDR_OFF_SIN_FAMILY], AF_INET

  mov   dword [rdi+SOCKADDR_OFF_SIN_ADDR], INADDR_ANY
  mov   qword [rdi+SOCKADDR_OFF_SA_ZERO], 0

  ; TODO: connect in its own func
  mov   rax, SYS_CONNECT
  mov   rdi, qword [rsp+0x8]
  lea   rsi, [rsp+0x10]
  mov   rdx, SOCKADDR_STRUCT_LEN
  syscall

  cmp   rax, 0
  jl    .error

  mov   rax, qword [rsp+0x8]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, SOCKADDR_STRUCT_LEN
  add   rsp, 0x10
  ret


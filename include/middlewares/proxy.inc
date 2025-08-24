section .bss
proxy_struct:
  _proxy_ip    resq 1
  _proxy_port  resq 1
proxy_struct_end:

section .data
PROXY_STRUCT_LEN equ proxy_struct_end - proxy_struct

PROXY_OFF_IP    equ 0x0
PROXY_OFF_PORT  equ PROXY_OFF_IP + 0x8

ROUND_ROBIN_IP          equ 0
WEIGHTED_ROUND_ROBIN_IP equ 1
IP_HASH                 equ 2

; keep track of the proxy index
proxy_idx dq 0

bad_gateway_message db "Bad Gateway: The remote server is unreachable. "
                    db "Forwarding failed because the connection could not be established.", NULL_CHAR

section .text
; configures a proxy middleware
; @param  rdi: pointer to the context struct
; @param  rsi: pointer to an array of proxy struct
; @param  rdx: load balancer algorithm
; @param  rcx: servers count
; @return rax: success code
proxy_middleware:
  sub   rsp, 0x30

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  cmp   rdx, 0
  jl    .error

  cmp   rcx, 0
  jle   .error

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> pointer to an array of proxy struct
  ; [rsp+0x10]  -> load balancer algorithm
  ; [rsp+0x18]  -> servers count
  ; [rsp+0x20]  -> server index
  ; [rsp+0x28]  -> file descriptor upstream server

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x10], rdx
  mov   qword [rsp+0x18], rcx

  ; connect to the end server   
  mov   rdi, [rsp]
  mov   rdx, [rsp+0x8]
  mov   rcx, qword [proxy_idx]
  mov   rsi, [rdx+8*rcx]
  call  connect_proxy
  cmp   rax, 0
  jl    .error

  ; save file descriptor
  mov   qword [rsp+0x28], rax

  mov   rdi, [rsp]
  mov   rsi, rax
  call  handle_proxy_request
  cmp   rax, 0
  jl    .error

  inc   qword [proxy_idx]
  mov   r9, qword [proxy_idx]
  cmp   r9, qword [rsp+0x18]
  jl    .skip_reset_idx

  mov   qword [proxy_idx], 0

.skip_reset_idx:
  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x30
  ret

; handles the request, sending it to the end server and returning the response to the 
; client
; @param  rdi: pointer to the context struct
; @param  rsi: file descriptor of the upstream server
; @return rax: return code
handle_proxy_request:
  sub   rsp, 0x30

  cmp   rdi, 0
  jl    .error

  cmp   rsi, 0
  jl    .error

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> file descriptor of the upstream server
  ; [rsp+0x10]  -> pointer to the server response
  ; [rsp+0x18]  -> server response length
  ; [rsp+0x20]  -> status code (int)
  ; [rsp+0x28]  -> status code (string)

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  ; send the request to the server
  call  send_request_to_server
  cmp   rax, 0
  jl    .error

  ; malloc response string
  mov   rdi, RESPONSE_MAX_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; receive response from the server
  mov   rdi, qword [rsp+0x8]
  mov   rsi, rax
  mov   rdx, RESPONSE_MAX_LEN
  call  receive_from_file
  cmp   rax, 0
  jl    .error
  
  mov   qword [rsp+0x18], rax

  ; add null char at the end of the request
  mov   rdi, [rsp+0x10]
  add   rdi, rax
  xor   rax, rax
  stosb

  ; send response to the client
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x10]
  mov   rdx, qword [rsp+0x18]
  call  send_response_from_proxy

  ; close connection with end server
  mov   rdi, qword [rsp+0x8]
  call  close_file

  ; parse response to get status code
  mov   rdi, [rsp+0x10]
  mov   rsi, SPACE
  call  find_next_char
  cmp   rax, 0
  jl    .error

  ; go after protocol
  mov   rsi, [rsp+0x10]
  add   rsi, rax
  inc   rsi

  ; copy status code
  lea   rdi, [rsp+0x28]
  mov   rcx, STATUS_CODE_LENGTH
  rep   movsb

  xor   rax, rax
  stosb

  lea   rdi, [rsp+0x28]
  call  stoi
  cmp   rax, 0
  jl    .error

  mov   qword [rsp+0x20], rax

  mov   rdi, [rsp]
  call  get_ctx_response
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, qword [rsp+0x20]
  call  set_response_status_code
  cmp   rax, 0
  jl    .error

  ; abort request
  mov   rdi, [rsp]
  call  ctx_abort

  ; free resources
  ; free response string
  mov   rdi, [rsp+0x10]
  call  free
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x30
  ret

; sends the server's response to the client
; @param  rdi: pointer to the context struct
; @param  rsi: pointer to the response
; @param  rdx: response length
; @return rax: return code
send_response_from_proxy:
  sub   rsp, 0x18

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  cmp   rdx, 0
  jle   .error

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> pointer to the response
  ; [rsp+0x10]  -> response length

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x10], rdx

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
  mov   rdi, rax
  mov   rsi, [rsp+0x8]
  mov   rdx, qword [rsp+0x10]
  call  write_file
  cmp   rax, 0
  jl    .error

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; sends the original request to the server
; @param  rdi: pointer to the context struct
; @param  rsi: file descriptor of the server
; @return rax: return code
send_request_to_server:
  sub   rsp, 0x20

  cmp   rdi, 0
  jl    .error

  cmp   rsi, 0
  jl    .error

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> file descriptor of the proxy
  ; [rsp+0x10]  -> pointer to request struct
  ; [rsp+0x18]  -> pointer to request string

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

 ; sends the request
  call  get_ctx_request
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  mov   rdi, rax
  call  get_request_str

  mov   [rsp+0x18], rax

  mov   rdi, [rsp+0x10]
  call  get_request_length
  cmp   rax, 0
  jl    .error

  ; sends request
  mov   rdi, qword [rsp+0x8]
  mov   rsi, [rsp+0x18]
  mov   rdx, rax
  call  write_file
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE
   
.return:
  add   rsp, 0x20
  ret

; configures the socket for the end server
; @param  rdi: pointer to the context struct
; @param  rsi: pointer to the proxy struct
; @return rax: file descriptor
connect_proxy:
  sub   rsp, 0x20
  sub   rsp, SOCKADDR_STRUCT_LEN

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> pointer to the proxy struct
  ; [rsp+0x10]  -> file descriptor
  ; [rsp+0x18]  -> sockaddr struct

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  ; create socket
  mov   rdi, AF_INET
  mov   rsi, SOCK_STREAM
  mov   rdx, 0
  call  create_socket
  cmp   rax, 0
  jl    .error

  mov   qword [rsp+0x10], rax

  ; set socket options
  mov   rdi, qword [rsp+0x10]
  mov   rsi, SOL_SOCKET
  mov   rdx, SO_REUSEPORT
  call  set_socket_option

  cmp   rax, 0
  jl    .error

  mov   rdi, qword [rsp+0x10]
  mov   rsi, SOL_SOCKET
  mov   rdx, SO_REUSEADDR
  call  set_socket_option

  cmp   rax, 0
  jl    .error

  ; convert port to network port
  mov   rsi, [rsp+0x8]
  mov   di, word [rsi+PROXY_OFF_PORT]
  call  htons

  lea   rdi, [rsp+0x18]
  mov   word [rdi+SOCKADDR_OFF_SIN_PORT], ax

  mov   word [rdi+SOCKADDR_OFF_SIN_FAMILY], AF_INET

  mov   rsi, [rsp+0x8]
  mov   rdi, [rsi+PROXY_OFF_IP]
  test  rdi, rdi
  jz    .ip_null 

  call  inet_addr
  cmp   rax, 0
  jl    .error

  lea   rdi, [rsp+0x18]
  mov   dword [rdi+SOCKADDR_OFF_SIN_ADDR], eax

  jmp   .zero

.ip_null:
  lea   rdi, [rsp+0x18]
  mov   dword [rdi+SOCKADDR_OFF_SIN_ADDR], INADDR_ANY

.zero:
  lea   rdi, [rsp+0x18]
  mov   qword [rdi+SOCKADDR_OFF_SA_ZERO], 0

  mov   rdi, qword [rsp+0x10]
  lea   rsi, [rsp+0x18]
  call  connect_fd

  cmp   rax, 0
  jl    .bad_gateway

  mov   rax, qword [rsp+0x10]
  jmp   .return

.bad_gateway:
  ; send bad gateway
  mov   rdi, [rsp]
  mov   rsi, BAD_GATEWAY
  lea   rdx, [bad_gateway_message]
  call  send_string

  cmp   rax, 0
  jl    .error

  ; abort request
  mov   rdi, [rsp]
  call  ctx_abort

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, SOCKADDR_STRUCT_LEN
  add   rsp, 0x20
  ret


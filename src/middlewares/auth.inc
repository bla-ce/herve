section .data

env_auth_key db "AUTH_ADMIN", NULL_CHAR

section .text


; middleware function to authenticate a request
; it only validates the request with a single username and password
; to test before implementing the authentication provider service
; @param  rdi: context
; @param  rsi: value auth
; @return rax: return code
svc_basic_middleware:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> valid auth
  ; [rsp+0x10]  -> decoded authorization header value

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  cmp   rdi, 0
  jle   .internal_server_error

  cmp   rsi, 0
  jle   .internal_server_error

  ; get request headers
  mov   rdi, [rsp]
  call  ctx_get_request
  cmp   rax, 0
  jl    .internal_server_error

  mov   rdi, rax
  call  request_get_headers
  cmp   rax, 0
  jl    .internal_server_error

  ; get authorization header value
  mov   rdi, rax
  mov   rsi, AUTHORIZATION_HEADER
  call  ht_get
  cmp   rax, 0
  jl    .unauthenticated

  mov   rdi, rax
  call  read_basic_auth
  cmp   rax, 0
  jl    .unauthenticated

  mov   [rsp+0x10], rax

  ; compare header value with expected auth
  mov   rdi, rax
  mov   rsi, [rsp+0x8]
  call  strcmp
  cmp   rax, 0
  jl    .unauthenticated
  cmp   rax, FALSE
  je    .unauthenticated

  ; free decoded value
  mov   rdi, [rsp+0x10]
  call  free
  cmp   rax, 0
  jl    .internal_server_error

  mov   rax, SUCCESS_CODE

  jmp   .return

.unauthenticated:
  ; return 401
  mov   rsi, UNAUTHORIZED
  jmp   .send_response

.internal_server_error:
  ; return 500
  mov   rsi, INTERNAL_SERVER_ERROR

.send_response:
  mov   rdi, [rsp]
  call  send_no_content

.error:
  mov   rdi, [rsp+0x10]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  ; abort request
  mov   rdi, [rsp]
  call  ctx_abort

  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; creates the authentication middleware to authenticate requests
; @param  rdi: pointer to the server
; @return rax: return code
svc_auth_middleware:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> pointer to the server
  ; [rsp+0x8]   -> valid auth
  ; [rsp+0x10]  -> pointer to the middleware struct

  mov   [rsp], rdi
  mov   qword [rsp+0x10], 0

  cmp   rdi, 0
  jle   .error

  mov   rdi, [env_ht]
  mov   rsi, env_auth_key
  call  ht_get
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; create middleware
  mov   rdi, svc_basic_middleware
  mov   rsi, [rsp+0x8]
  mov   rdx, NO_ARG
  mov   rcx, NO_ARG
  mov   r9, FALSE
  call  create_middleware
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; add middleware to struct
  mov   rdi, [rsp]
  mov   rsi, NO_ARG
  mov   rdx, [rsp+0x10]
  call  add_middleware
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rdi, [rsp+0x10]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

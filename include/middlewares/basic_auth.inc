section .data

section .text

; validates the request with basic auth
; TODO: should you send username and password as arg?
; @param  rdi: pointer to the context struct
; @param  rsi: username
; @param  rdx: password
; @return rax: success code
basic_auth_middleware:
  sub   rsp, 0x28

  ; STACK USAGE
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> valid username
  ; [rsp+0x10]  -> valid password
  ; [rsp+0x18]  -> Authorization header
  ; [rsp+0x20]  -> username length

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   qword [rsp+0x18], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  cmp   rdx, 0
  jle   .error

  mov   rdi, [rsp]
  call  ctx_get_request
  cmp   rax, 0
  jl    .error

  ; get authorization header
  mov   rdi, rax
  call  request_get_headers
  cmp   rax, 0
  jl    .error

  ; decode auth
  mov   rdi, rax
  lea   rsi, [AUTHORIZATION_HEADER]
  call  get_header_value
  cmp   rax, 0
  jl    .unauthorized

  mov   rdi, rax
  call  read_basic_auth
  cmp   rax, 0
  jl    .unauthorized

  mov   [rsp+0x18], rax

  ; check username length
  mov   rdi, [rsp+0x18]
  mov   rsi, COLON
  call  find_next_char
  cmp   rax, 0
  jl    .unauthorized

  mov   [rsp+0x20], rax

  ; compare username
  mov   rdi, [rsp+0x18]
  mov   rsi, [rsp+0x8]
  mov   rdx, [rsp+0x20]
  call  strncmp
  cmp   rax, 0
  jle   .unauthorized

  ; go past ':'
  mov   rdi, [rsp+0x18]
  add   rdi, [rsp+0x20]
  inc   rdi
  mov   rsi, [rsp+0x10]
  call  strcmp
  cmp   rax, 0
  jle   .unauthorized

  mov   rdi, [rsp+0x18]
  call  free
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.unauthorized:
  mov   rdi, [rsp]
  mov   rsi, UNAUTHORIZED
  call  send_no_content

  ; abort ctx
  mov   rdi, [rsp]
  call  ctx_abort

.error:
  mov   rdi, [rsp+0x18]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x28
  ret

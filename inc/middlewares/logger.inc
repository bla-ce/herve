section .data
  logger_delim db " | ", NULL_CHAR

section .text

; logs information about a request
; format: [INFO] timestamp | ip | status_code | method | url
; @param  rdi: pointer to the context struct
; @param  rsi: pointer to the logan struct
; @return rax: return code
log_ctx:
  sub   rsp, 0x18

  ; *** STACK USAGE *** ;
  ; [rsp]     -> pointer to the context struct
  ; [rsp+0x8] -> pointer to the logan struct

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x10], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   rdi, logger_delim
  call  boeuf_create
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; get the client
  mov   rdi, [rsp]
  call  get_ctx_client
  cmp   rax, 0
  jl    .error

  ; get ip 
  mov   rdi, rax
  call  get_client_ip
  cmp   rax, 0
  jl    .error

  ; create boeuf buffer
  mov   rdi, [rsp+0x10]
  mov   rsi, rax
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  mov   rdi, [rsp+0x10]
  mov   rsi, logger_delim
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; get status code
  mov   rdi, [rsp]
  call  get_ctx_response
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  get_response_status_code
  cmp   rax, 0
  jl    .error

  ; get string status code
  mov   rdi, rax
  call  get_status_string
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x10]
  mov   rsi, rax
  call  boeuf_append
  cmp   rax, 0
  jl    .error
  
  mov   [rsp+0x10], rax

  mov   rdi, [rsp+0x10]
  mov   rsi, logger_delim
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; get method
  mov   rdi, [rsp]
  call  get_ctx_request
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  get_request_method
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x10]
  mov   rsi, rax
  call  boeuf_append
  cmp   rax, 0
  jl    .error
  
  mov   [rsp+0x10], rax

  mov   rdi, [rsp+0x10]
  mov   rsi, logger_delim
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; get url
  mov   rdi, [rsp]
  call  get_ctx_request
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  get_request_url
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x10]
  mov   rsi, rax
  call  boeuf_append
  cmp   rax, 0
  jl    .error
  
  mov   [rsp+0x10], rax

  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  log_infoln
  cmp   rax, 0
  jl    .error

  ; free boeuf buffer
  mov   rdi, [rsp+0x10]
  call  boeuf_free
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rdi, [rsp+0x10]
  test  rdi, rdi
  jz    .no_free

  call  boeuf_free
  cmp   rax, 0
  jl    .error

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret


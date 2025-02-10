section .data
  ctx_struct:
    ctx_client      dq 0
    ctx_server      dq 0
    ctx_proto       dq 0
    ctx_request     dq 0
    ctx_response    dq 0
    ctx_start_time  dq 0
    ctx_end_time    dq 0
  ctx_struct_end:

  CTX_STRUCT_LEN equ ctx_struct_end - ctx_struct

  CTX_OFF_CLIENT      equ 0x0
  CTX_OFF_SERVER      equ CTX_OFF_CLIENT + 0x8
  CTX_OFF_PROTO       equ CTX_OFF_SERVER + 0x8
  CTX_OFF_REQUEST     equ CTX_OFF_PROTO + 0x8
  CTX_OFF_RESPONSE    equ CTX_OFF_REQUEST + 0x8
  CTX_OFF_START_TIME  equ CTX_OFF_RESPONSE + 0x8
  CTX_OFF_END_TIME    equ CTX_OFF_START_TIME + 0x8

  HTTP_1_1 db "HTTP/1.1", NULL_CHAR

section .text
; closes the connection
; @param  rdi: pointer to context struct
; @return rax: return code
ctx_close:
  call  get_ctx_client

  cmp   rax, 0
  jl    .error

  call  get_client_fd
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rax, SYS_CLOSE
  syscall

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; initialise context structure
; @param  rdi: pointer to server
; @return rax: pointer to ctx struct
ctx_init:
  sub   rsp, 0x28

  mov   [rsp], rdi

  ; malloc ctx
  mov   rdi, CTX_STRUCT_LEN
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; malloc request
  mov   rdi, REQ_STRUCT_LEN
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x10]
  call  set_ctx_request

  cmp   rax, 0
  jl    .error

  ; malloc client
  mov   rdi, CLIENT_STRUCT_LEN
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x18]
  call  set_ctx_client

  cmp   rax, 0
  jl    .error

  ; init response
  call  response_init
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x20]
  call  set_ctx_response

  cmp   rax, 0
  jl    .error

  ; set server
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp]
  call  set_ctx_server

  cmp   rax, 0
  jl    .error

  ; return context
  mov   rax, [rsp+0x8]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x28
  ret

; returns the pointer to the request of the context
; @param  rdi: pointer to context
; @return rax: pointer to the request
get_ctx_request:
  cmp   rdi, 0
  jl    .error

  mov   rax, [rdi+CTX_OFF_REQUEST]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the pointer to the response of the context
; @param  rdi: pointer to context
; @return rax: pointer to the response
get_ctx_response:
  cmp   rdi, 0
  jl    .error

  mov   rax, [rdi+CTX_OFF_RESPONSE]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the start time of the context
; @param  rdi: pointer to context
; @param  rsi: timestamp
; @return rax: return code
set_ctx_start_time:
  cmp   rdi, 0
  jl    .error

  cmp   rsi, 0
  jl    .error

  mov   qword [rdi+CTX_OFF_START_TIME], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the end time of the context
; @param  rdi: pointer to context
; @param  rsi: timestamp
; @return rax: return code
set_ctx_end_time:
  mov   rdi, 0
  jl    .error

  mov   rsi, 0
  jl    .error

  mov   qword [rdi+CTX_OFF_END_TIME], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the request struct of the context
; @param  rdi: pointer to context
; @param  rsi: pointer to request
; @return rax: return code
set_ctx_request:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   [rdi+CTX_OFF_REQUEST], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the client struct of the context
; @param  rdi: pointer to context
; @param  rsi: pointer to client
; @return rax: return code
set_ctx_client:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   [rdi+CTX_OFF_CLIENT], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the server struct of the context
; @param  rdi: pointer to context
; @param  rsi: pointer to server
; @return rax: return code
set_ctx_server:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   [rdi+CTX_OFF_SERVER], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the response struct of the context
; @param  rdi: pointer to context
; @param  rsi: pointer to response
; @return rax: return code
set_ctx_response:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   [rdi+CTX_OFF_RESPONSE], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the client struct of the context
; @param  rdi: pointer to context
; @return rax: pointer to client
get_ctx_client:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+CTX_OFF_CLIENT]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the server struct of the context
; @param  rdi: pointer to context
; @return rax: pointer to server
get_ctx_server:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+CTX_OFF_SERVER]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret


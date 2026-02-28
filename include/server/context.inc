section .bss
context_t:
  .client     resq 1
  .server     resq 1
  .proto      resq 1
  .request    resq 1
  .response   resq 1
  .start_time resq 1
  .end_time   resq 1
  .aborted    resq 1
  .done       resq 1
context_t_end:

section .data

CTX_STRUCT_LEN equ context_t_end - context_t

CTX_OFF_CLIENT      equ 0x0
CTX_OFF_SERVER      equ CTX_OFF_CLIENT + 0x8
CTX_OFF_PROTO       equ CTX_OFF_SERVER + 0x8
CTX_OFF_REQUEST     equ CTX_OFF_PROTO + 0x8
CTX_OFF_RESPONSE    equ CTX_OFF_REQUEST + 0x8
CTX_OFF_START_TIME  equ CTX_OFF_RESPONSE + 0x8
CTX_OFF_END_TIME    equ CTX_OFF_START_TIME + 0x8
CTX_OFF_ABORTED     equ CTX_OFF_END_TIME + 0x8
CTX_OFF_DONE        equ CTX_OFF_ABORTED + 0x8

HTTP_1_1 db "HTTP/1.1", NULL_CHAR

section .text
; initializes and processes a new request context
; @param  rdi: pointer to the server
; @param  rsi: pointer to client struct
; @return rax: return code
ctx_process_request:
  sub   rsp, 0x38

  ; STACK USAGE
  ; [rsp]       -> pointer to the server struct
  ; [rsp+0x8]   -> pointer to the client struct
  ; [rsp+0x10]  -> pointer to the context struct
  ; [rsp+0x18]  -> pointer to the root group
  ; [rsp+0x20]  -> pointer to the route struct
  ; [rsp+0x28]  -> pointer to the route group
  ; [rsp+0x30]  -> pointer to the route handler

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x18], 0
  mov   qword [rsp+0x28], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  call  ctx_init
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; set request start time
  call  time_now_ns
  cmp   rax, 0
  jl    .internal_server_error

  mov   rdi, [rsp+0x10]
  mov   rsi, rax
  call  ctx_set_start_time
  cmp   rax, 0
  jl    .internal_server_error

  ; receive and parse the request
  mov   rdi, [rsp+0x10]
  call  handle_request
  cmp   rax, 0
  jl    .internal_server_error

  mov   rdi, [rsp+0x10]
  call  ctx_is_aborted
  cmp   rax, 0
  jl    .internal_server_error
  cmp   rax, TRUE
  je    .post_request_middlewares

  ; get root group
  mov   rdi, [rsp]
  call  server_get_root_group
  cmp   rax, 0
  jl    .internal_server_error

  mov   [rsp+0x18], rax

  ; run root pre request middleware
  mov   rdi, [rsp+0x18]
  mov   rsi, [rsp+0x10]
  mov   rdx, FALSE
  call  middleware_run_chain
  cmp   rax, 0
  ; here, we send a 500 because it's a server error,
  ; not middleware error. A middleware error will just abort
  ; the ctx, check below
  jl    .internal_server_error

  mov   rdi, [rsp+0x10]
  call  ctx_is_aborted
  cmp   rax, 0
  jl    .internal_server_error
  cmp   rax, TRUE
  je    .post_request_middlewares

  ; verify request
  mov   rdi, [rsp+0x10]
  call  verify_request
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  ; check if request has been aborted (404, 405 etc...)
  mov   rdi, [rsp+0x10]
  call  ctx_is_aborted
  cmp   rax, 0
  jl    .internal_server_error
  cmp   rax, TRUE
  je    .post_request_middlewares

  ; get route group
  mov   rdi, [rsp+0x20]
  call  route_get_group
  cmp   rax, 0
  jl    .internal_server_error

  mov   [rsp+0x28], rax

  ; run route pre request middleware
  mov   rdi, [rsp+0x28]
  mov   rsi, [rsp+0x10]
  mov   rdx, FALSE
  call  middleware_run_chain
  cmp   rax, 0
  jl    .internal_server_error

  ; check if request has been aborted (404, 405 etc...)
  mov   rdi, [rsp+0x10]
  call  ctx_is_aborted
  cmp   rax, 0
  jl    .internal_server_error
  cmp   rax, TRUE
  je    .post_request_middlewares

  ; get route handler
  mov   rdi, [rsp+0x20]
  call  route_get_callback
  cmp   rax, 0
  jl    .internal_server_error

  mov   [rsp+0x30], rax

  mov   rdi, [rsp+0x20]
  call  route_get_arg1
  cmp   rax, 0
  jl    .internal_server_error

  ; run route handler
  mov   rdi, [rsp+0x10]
  mov   rsi, rax
  mov   rax, [rsp+0x30]
  call  rax
  cmp   rax, 0
  jl    .internal_server_error

.post_request_middlewares:
  ; set request end time
  call  time_now_ns
  cmp   rax, 0
  jl    .internal_server_error

  mov   rdi, [rsp+0x10]
  mov   rsi, rax
  call  ctx_set_end_time
  cmp   rax, 0
  jl    .internal_server_error

  ; run root post request middleware
  mov   rdi, [rsp+0x18]
  mov   rsi, [rsp+0x10]
  mov   rdx, TRUE
  call  middleware_run_chain
  cmp   rax, 0
  ; we can't return 500 because we don't know what's
  ; been sent before
  jl    .ctx_free

  ; run route post request middleware
  mov   rdi, [rsp+0x28]
  mov   rsi, [rsp+0x10]
  mov   rdx, TRUE
  call  middleware_run_chain

  jmp   .ctx_free

.internal_server_error:
  mov   rdi, [rsp+0x10]
  mov   rsi, INTERNAL_SERVER_ERROR
  call  send_no_content

  ; TODO: should return a string here

.ctx_free:
  mov   rdi, [rsp+0x10]
  call  ctx_free

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rdi, [rsp+0x10]
  test  rdi, rdi
  jz    .no_free

  call  ctx_free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x38
  ret

; frees the resources associated to the context
; @param  rdi: pointer to the context
ctx_free:
  sub   rsp, 0x8

  mov   [rsp], rdi

  ; get response
  mov   rdi, [rsp]
  call  ctx_get_response
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  free_response
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  call  ctx_get_request
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  free_request
  cmp   rax, 0
  jl    .error

  ; free ctx struct
  mov   rdi, [rsp]
  call  free
  cmp   rax, 0
  jl    .error

  jmp   .return

.error:
  ; TODO: what?

.return:
  add   rsp, 0x8
  ret

; abort context, close the connection
; @param  rdi: ctx
; @return rax: return code
ctx_abort:
  cmp   rdi, 0
  jle   .error

  mov   qword [rdi+CTX_OFF_ABORTED], TRUE
  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; check if ctx has been aborted
; @param  rdi: ctx
; @return rax: is_aborted
ctx_is_aborted:
  cmp   rdi, 0
  jle   .error

  mov   rax, qword [rdi+CTX_OFF_ABORTED]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; initialise context structure
; @param  rdi: pointer to server struct
; @param  rsi: pointer to client struct
; @return rax: pointer to ctx struct
ctx_init:
  sub   rsp, 0x28

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  ; malloc ctx
  mov   rdi, CTX_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; malloc request
  mov   rdi, REQ_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  mov   rdi, [rsp+0x18]
  call  request_init
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp+0x18]
  call  ctx_set_request
  cmp   rax, 0
  jl    .error

  ; set client fd
  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp+0x8]
  call  ctx_set_client
  cmp   rax, 0
  jl    .error

  ; init response
  call  response_init
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp+0x20]
  call  ctx_set_response
  cmp   rax, 0
  jl    .error

  ; set server
  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp]
  call  ctx_set_server
  cmp   rax, 0
  jl    .error

  ; set aborted to 0
  mov   rdi, [rsp+0x10]
  mov   qword [rdi+CTX_OFF_ABORTED], 0
  mov   qword [rdi+CTX_OFF_DONE], 0

  ; return context
  mov   rax, [rsp+0x10]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x28
  ret

; retusn the latency of an ended and processes request
; it will return an error if the request hasn't been processed
; @param  rdi: pointer to the context struct
; @return rax: return the latency (ns)
ctx_get_request_latency:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> pointer to the context struct
  ; [rsp+0x8] -> start timestamp

  mov   [rsp], rdi

  cmp   rdi, 0
  jl    .error

  ; get start time
  mov   rdi, [rsp]
  call  ctx_get_start_time
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, [rsp]
  call  ctx_get_end_time
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  sub   rax, rdi

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; returns the pointer to the request of the context
; @param  rdi: pointer to context
; @return rax: pointer to the request
ctx_get_request:
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
ctx_get_response:
  cmp   rdi, 0
  jl    .error

  mov   rax, [rdi+CTX_OFF_RESPONSE]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the start time of the context
; @param  rdi: pointer to context
; @return rax: timestamp
ctx_get_start_time:
  cmp   rdi, 0
  jl    .error

  mov   rax, qword [rdi+CTX_OFF_START_TIME]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the start time of the context
; @param  rdi: pointer to context
; @param  rsi: timestamp
; @return rax: return code
ctx_set_start_time:
  cmp   rdi, 0
  jl    .error

  cmp   rsi, 0
  jl    .error

  mov   [rdi+CTX_OFF_START_TIME], rsi

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
ctx_set_end_time:
  cmp   rdi, 0
  jl    .error

  cmp   rsi, 0
  jl    .error

  mov   [rdi+CTX_OFF_END_TIME], rsi

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the end time of the context
; @param  rdi: pointer to context
; @return rax: timestamp
ctx_get_end_time:
  cmp   rdi, 0
  jl    .error

  mov   rax, qword [rdi+CTX_OFF_END_TIME]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the request struct of the context
; @param  rdi: pointer to context
; @param  rsi: pointer to request
; @return rax: return code
ctx_set_request:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0 ; ok to have zero request
  jl    .error

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
ctx_set_client:
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
ctx_set_server:
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
ctx_set_response:
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
ctx_get_client:
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
ctx_get_server:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+CTX_OFF_SERVER]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

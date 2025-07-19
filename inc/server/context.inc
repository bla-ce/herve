section .bss
ctx_struct:
  ctx_client      resq 1
  ctx_server      resq 1
  ctx_proto       resq 1
  ctx_request     resq 1
  ctx_response    resq 1
  ctx_start_time  resq 1
  ctx_end_time    resq 1
  ctx_aborted     resq 1
  ctx_done        resq 1
ctx_struct_end:

section .data
  CTX_STRUCT_LEN equ ctx_struct_end - ctx_struct

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
; log the duration of the context
; @param  rdi: pointer to the logger struct
; @param  rsi: pointer to the context struct
; @return rax: return code
log_ctx_duration:
  sub   rsp, 0x20
  sub   rsp, DURATION_MAX_SIZE

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the logger struct
  ; [rsp+0x8]   -> pointer to the context struct
  ; [rsp+0x10]  -> ctx start time
  ; [rsp+0x18]  -> duration
  ; [rsp+0x20]  -> string duration (size 0x10)

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  ; get end and start time
  mov   rdi, [rsp+0x8]
  call  get_ctx_start_time

  cmp   rax, 0
  jl    .error

  mov   qword [rsp+0x10], rax
  mov   qword [rsp+0x20], 0

  mov   rdi, [rsp+0x8]
  call  get_ctx_end_time

  cmp   rax, 0
  jl    .error

  ; subtract end and start time
  mov   rbx, qword [rsp+0x10]
  sub   rax, rbx

  cmp   rax, 0
  jl    .error

  ; save duration
  mov   qword [rsp+0x18], rax

  lea   rsi, [rsp+0x20]
  add   rsi, DURATION_MAX_SIZE

  ; add null char at the end
  dec   rsi
  mov   byte [rsi], NULL_CHAR
  dec   rsi

  ; print micro second backwards
  mov   byte [rsi], LOWERCASE_S
  dec   rsi
  mov   byte [rsi], MICRO_SECOND
  dec   rsi
  mov   byte [rsi], MICRO_FIRST

  mov   rax, qword [rsp+0x18]

  xor   rdx, rdx
  mov   rcx, 1000
  div   rcx
  
.loop:
  dec   rsi

  xor   rdx, rdx
  mov   rcx, 0xA  ; divisor, dividend in rax
  div   rcx       ; quotient in rax and remainder in edx

  add   rdx, ZERO_CHAR

  mov   byte [rsi], dl

  cmp   rax, 0
  jne   .loop

  ; print duration
  mov   rdi, [rsp]
  ; rsi already populated
  call  log

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, DURATION_MAX_SIZE
  add   rsp, 0x20
  ret

; log the current context
; @param  rdi: pointer to context
; @param  rsi: pointer to the server logger
log_ctx:
  sub   rsp, 0x18

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  ; [LOG] 
  mov   rdi, [rsp+0x8]
  lea   rsi, [log_pre]
  call  log

  mov   rdi, [rsp]
  call  get_ctx_start_time

  mov   rax, 0
  jl    .error

  ; log time
  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  log_time

  mov   rdi, [rsp+0x8]
  call  log_delimiter

  ; log client ip
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp]
  call  log_client_ip

  mov   rdi, [rsp+0x8]
  call  log_delimiter

  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp]
  call  log_ctx_duration

  mov   rdi, [rsp+0x8]
  call  log_delimiter

  ; log status code
  ; get context response
  mov   rdi, [rsp]
  call  get_ctx_response

  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  get_response_status_code

  cmp   rax, 0
  jl    .error

  ; get response status code
  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  log_status_code

  mov   rdi, [rsp+0x8]
  call  log_delimiter

  ; log method
  mov   rdi, [rsp]
  call  get_ctx_request
  
  cmp   rax, 0
  jl    .error
  je    .no_request

  mov   [rsp+0x10], rax

  mov   rdi, rax
  call  get_request_method

  cmp   rax, 0
  jl    .error
  je    .no_request

  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  log

  mov   rdi, [rsp+0x8]
  mov   rsi, SPACE
  call  log_char

  ; log url
  mov   rdi, [rsp+0x10]
  call  get_request_url

  cmp   rax, 0
  jl    .error
  je    .no_request

  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  log

.no_request:
  mov   rdi, [rsp+0x8]
  mov   rsi, LINE_FEED
  call  log_char

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; initializes and processes a new request context
; @param  rdi: pointer to the server  
; @param  rsi: pointer to client struct
; @return rax: return code
handle_ctx:
  sub   rsp, 0x20

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  mov   rdi, [rsp] 
  mov   rsi, [rsp+0x8]
  call  ctx_init
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  call  get_curr_ns
  cmp   rax, 0
  jl   .log_request 

  mov   rdi, [rsp+0x10]
  mov   rsi, rax
  call  set_ctx_start_time
  cmp   rax, 0
  jl   .log_request 

  mov   rdi,  [rsp+0x10]
  call  handle_request
  cmp   rax, 0
  jl    .error

  ; check if ctx has been aborted
  mov   rdi, [rsp+0x10]
  call  is_ctx_aborted
  cmp   rax, TRUE
  je    .log_request

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x10]
  call  run_middlewares

; check if ctx has been aborted
  mov   rdi, [rsp+0x10]
  call  is_ctx_aborted
  cmp   rax, TRUE
  je    .log_request

  ; verify request
  mov   rdi, [rsp+0x10]
  call  verify_request
  cmp   rax, 0
  jl   .log_request

  mov   [rsp+0x18], rax

  ; check if ctx has been aborted
  mov   rdi, [rsp+0x10]
  call  is_ctx_aborted
  cmp   rax, TRUE
  je    .log_request

  ; check if route has a callback
  mov   rdi, [rsp+0x18]
  call  get_route_callback
  cmp   rax, 0
  je   .done

  mov   rdi, [rsp+0x10]
  call  rax

  mov   rdi, [rsp+0x10]
  cmp   qword [rdi+CTX_OFF_DONE], 1
  je    .log_request

.done:
  mov   rdi, [rsp+0x10]
  mov   rsi, OK
  call  send_no_content

.log_request:
  ; set end timer
  call  get_curr_ns
  cmp   rax, 0
  jl   .log_request 

  ; if yes, compute the difference
  mov   rdi, [rsp+0x10]
  mov   rsi, rax
  call  set_ctx_end_time
  cmp   rax, 0
  jl    .error

  ; get server logger
  mov   rdi, [rsp]
  call  get_server_logger
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x10]
  mov   rsi, rax
  call  log_ctx

.free_ctx:
  mov   rdi, [rsp+0x10]
  call  free_ctx
  
  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rdi, [rsp+0x10]
  test  rdi, rdi
  jz    .no_free

  call  free_ctx

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret

; frees the resources associated to the context
; @param  rdi: pointer to the context
free_ctx:
  sub   rsp, 0x8

  mov   [rsp], rdi

  ; get response
  mov   rdi, [rsp]
  call  get_ctx_response
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  free_response
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  call  get_ctx_request
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
is_ctx_aborted:
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
  call  set_ctx_request

  cmp   rax, 0
  jl    .error
   
  ; set client fd
  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp+0x8]
  call  set_ctx_client

  cmp   rax, 0
  jl    .error

  ; init response
  call  response_init
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp+0x20]
  call  set_ctx_response

  cmp   rax, 0
  jl    .error

  ; set server
  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp]
  call  set_ctx_server

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

; returns the start time of the context
; @param  rdi: pointer to context
; @return rax: timestamp
get_ctx_start_time:
  cmp   rdi, 0
  jl    .error

  mov   rax, qword [rdi+CTX_OFF_START_TIME]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the end time of the context
; @param  rdi: pointer to context
; @return rax: timestamp
get_ctx_end_time:
  cmp   rdi, 0
  jl    .error

  mov   rax, qword [rdi+CTX_OFF_END_TIME]

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
  cmp   rdi, 0
  jl    .error

  cmp   rsi, 0
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


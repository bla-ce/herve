section .data
  logger_delim db " | ", NULL_CHAR

  ; offsets
  LOGGER_STATUS_CODE_OFF  equ 00010000b
  LOGGER_URL_OFF          equ 00001000b
  LOGGER_METHOD_OFF       equ 00000100b
  LOGGER_IP_OFF           equ 00000010b
  LOGGER_LATENCY_OFF      equ 00000001b ; unused yet
  ; | PAD | S | U | M | I | L |

section .text

; returns a middleware struct with log_ctx
; @return rax: pointer to the middleware struct
get_logger_middleware:
  ; get default logger
  call  logan_init
  cmp   rax, 0
  jl    .error

  ; create the middleware
  mov   rdi, log_ctx
  mov   rsi, rax
  mov   rdx, 0xFF
  mov   rcx, 0
  mov   r9, TRUE
  call  create_middleware
  cmp   rax, 0
  jl    .error

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; logs information about a request
; format: [INFO] timestamp | ip | status_code | method | url
; @param  rdi: pointer to the context struct
; @param  rsi: pointer to the logan struct
; @param  rdx: flags
; @return rax: return code
log_ctx:
  sub   rsp, 0x20

  ; STACK USAGE
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> pointer to the logan struct
  ; [rsp+0x10]  -> flags
  ; [rsp+0x18]  -> pointer to the boeuf buffer

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   qword [rsp+0x18], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  cmp   rdx, 0
  jle   .return

  ; create boeuf buffer
  mov   rdi, logger_delim
  call  boeuf_create
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  mov   rdi, [rsp+0x10]
  test  rdi, LOGGER_IP_OFF
  jz    .skip_ip

  ; get the client
  mov   rdi, [rsp]
  call  get_ctx_client
  cmp   rax, 0
  jl    .error

  ; get ip
  mov   rdi, rax
  call  client_get_ip
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x18]
  mov   rsi, rax
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  mov   rdi, [rsp+0x18]
  mov   rsi, logger_delim
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

.skip_ip:
  mov   rdi, [rsp+0x10]
  test  rdi, LOGGER_STATUS_CODE_OFF
  jz    .skip_status_code

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

  mov   rdi, [rsp+0x18]
  mov   rsi, rax
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  mov   rdi, [rsp+0x18]
  mov   rsi, logger_delim
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

.skip_status_code:
  mov   rdi, [rsp+0x10]
  test  rdi, LOGGER_METHOD_OFF
  jz    .skip_method

  ; get method
  mov   rdi, [rsp]
  call  get_ctx_request
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  get_request_method
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x18]
  mov   rsi, rax
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  mov   rdi, [rsp+0x18]
  mov   rsi, logger_delim
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

.skip_method:
  mov   rdi, [rsp+0x10]
  test  rdi, LOGGER_URL_OFF
  jz    .skip_url

  ; get url
  mov   rdi, [rsp]
  call  get_ctx_request
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  get_request_url
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x18]
  mov   rsi, rax
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

.skip_url:
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x18]
  call  log_infoln
  cmp   rax, 0
  jl    .error

  ; free boeuf buffer
  mov   rdi, [rsp+0x18]
  call  boeuf_free
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rdi, [rsp+0x18]
  test  rdi, rdi
  jz    .no_free

  call  boeuf_free
  cmp   rax, 0
  jl    .error

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret

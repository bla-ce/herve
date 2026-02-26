section .data
  logger_delim db " | ", NULL_CHAR

  ; offsets
  LOGGER_STATUS_CODE_OFF  equ 00010000b
  LOGGER_URL_OFF          equ 00001000b
  LOGGER_METHOD_OFF       equ 00000100b
  LOGGER_IP_OFF           equ 00000010b
  LOGGER_LATENCY_OFF      equ 00000001b
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
  call  middleware_create
  cmp   rax, 0
  jl    .error

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; logs information about a request
; format: [INFO] status code | ip | method | url
; @param  rdi: pointer to the context struct
; @param  rsi: pointer to the logan struct
; @param  rdx: flags
; @return rax: return code
log_ctx:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> pointer to the logan struct
  ; [rsp+0x10]  -> flags

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  cmp   rdx, 0
  jle   .return

  ; log the prefix
  mov   rdi, [rsp+0x8]
  mov   rsi, logger_delim
  call  log_info
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x10]
  test  rdi, LOGGER_IP_OFF
  jz    .skip_ip

  ; get the client
  mov   rdi, [rsp]
  call  ctx_get_client
  cmp   rax, 0
  jl    .error

  ; get ip
  mov   rdi, rax
  call  client_get_ip
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  log       ; log the prefix
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, logger_delim
  call  log       ; log without prefix
  cmp   rax, 0
  jl    .error

.skip_ip:
  mov   rdi, [rsp+0x10]
  test  rdi, LOGGER_STATUS_CODE_OFF
  jz    .skip_status_code

  ; get status code
  mov   rdi, [rsp]
  call  ctx_get_response
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

  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  log
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, logger_delim
  call  log
  cmp   rax, 0
  jl    .error

.skip_status_code:
  mov   rdi, [rsp+0x10]
  test  rdi, LOGGER_METHOD_OFF
  jz    .skip_method

  ; get method
  mov   rdi, [rsp]
  call  ctx_get_request
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  get_request_method
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  log
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, logger_delim
  call  log
  cmp   rax, 0
  jl    .error

.skip_method:
  mov   rdi, [rsp+0x10]
  test  rdi, LOGGER_URL_OFF
  jz    .skip_url

  ; get url
  mov   rdi, [rsp]
  call  ctx_get_request
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  get_request_url
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  log
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, logger_delim
  call  log
  cmp   rax, 0
  jl    .error

.skip_url:
  mov   rdi, [rsp+0x10]
  test  rdi, LOGGER_LATENCY_OFF
  jz    .skip_latency

  ; get request latency
  mov   rdi, [rsp]
  call  ctx_get_request_latency
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  log_latency
  cmp   rax, 0
  jl    .error

.skip_latency:
  mov   rdi, [rsp+0x8]
  mov   rsi, logger_delim
  call  logln
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; logs the latency
; @param  rdi: pointer to the logger
; @param  rsi: latency in ns
; @return rax: return code
log_latency:
  sub   rsp, 0x18
  sub   rsp, UNSIGNED_LONG_MAX_STR_SIZE

  ; STACK USAGE
  ; [rsp]       -> pointer to the logger struct
  ; [rsp+0x8]   -> latency
  ; [rsp+0x10]  -> latency unit
  ; [rsp+0x18]  -> str(timestamp)

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x10], ns_str  ; by default, latency is ns

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  ; check units, if latency is greater than 999ns, print us
  cmp   qword [rsp+0x8], 1000
  jl    .log

.get_us:
  ; divide by 1000 and get us
  xor   rdx, rdx
  mov   rax, [rsp+0x8]
  mov   rbx, 1000
  div   rbx

  mov   [rsp+0x8], rax

  mov   qword [rsp+0x10], us_str

  cmp   qword [rsp+0x8], 1000
  jl    .log

.get_ms:
  ; divide by 1000 and get ms
  xor   rdx, rdx
  mov   rax, [rsp+0x8]
  mov   rbx, 1000
  div   rbx

  mov   [rsp+0x8], rax

  mov   qword [rsp+0x10], ms_str

  cmp   qword [rsp+0x8], 1000
  jl    .log

.get_s:
  ; divide by 1000 and get s
  xor   rdx, rdx
  mov   rax, [rsp+0x8]
  mov   rbx, 1000
  div   rbx

  mov   [rsp+0x8], rax

  mov   qword [rsp+0x10], s_str

.log:
  ; convert number to string
  mov   rdi, [rsp+0x8]
  lea   rsi, [rsp+0x18]
  mov   rdx, UNSIGNED_LONG_MAX_STR_SIZE
  call  itoa
  cmp   rax, 0
  jl    .error

  ; print latency
  mov   rdi, [rsp]
  mov   rsi, rax
  call  log
  cmp   rax, 0
  jl    .error

  ; print unit
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x10]
  call  log
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, UNSIGNED_LONG_MAX_STR_SIZE
  add   rsp, 0x18
  ret

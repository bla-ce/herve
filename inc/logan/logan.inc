section .bss

logan_fmt_struct:
  .colors       resq 1    ; enable or disable colors
  .timestamp    resq 1    ; enable or disable timestamp
  .txt_max_len  resq 1    ; max length of the log message
logan_fmt_struct_end:

logan_struct:
  logan_format  resq 1  ; pointer to the format struct
  logan_output  resq 1  ; file descriptor of the output
  logan_level   resq 1  ; minimum level to be printed
logan_struct_end:

section .data

LOGAN_STRUCT_LEN      equ logan_struct_end - logan_struct
LOGAN_FMT_STRUCT_LEN  equ logan_fmt_struct_end - logan_fmt_struct

; logan offsets
LOGAN_OFF_FMT equ 0x0
LOGAN_OFF_OUT equ LOGAN_OFF_FMT + 0x8
LOGAN_OFF_LVL equ LOGAN_OFF_OUT + 0x8

LOGAN_FMT_OFF_COLORS      equ 0x0
LOGAN_FMT_OFF_TIMESTAMP   equ LOGAN_FMT_OFF_COLORS + 0x8
LOGAN_FMT_OFF_TXT_MAX_LEN equ LOGAN_FMT_OFF_TIMESTAMP + 0x8

LOGAN_DEFAULT_TXT_MAX_LEN equ 0xFF

; log levels constants
LOG_TRACE equ 0
LOG_DEBUG equ 1
LOG_LOG   equ 2
LOG_INFO  equ 3
LOG_WARN  equ 4
LOG_ERROR equ 5
LOG_FATAL equ 6

; log levels prefixes
log_prefixes:
  trace_prefix  db "[TRACE] ", NULL_CHAR
  debug_prefix  db "[DEBUG] ", NULL_CHAR
  log_prefix    db "[LOG]   ", NULL_CHAR
  info_prefix   db "[INFO]  ", NULL_CHAR
  warn_prefix   db "[WARN]  ", NULL_CHAR
  error_prefix  db "[ERROR] ", NULL_CHAR
  fatal_prefix  db "[FATAL] ", NULL_CHAR
log_prefixes_end:

LOG_PREFIX_SIZE equ error_prefix - fatal_prefix

section .text
; initializes a logan struct with default value
; @return rax: pointer to the logan struct
logan_init:
  sub   rsp, 0x8

  ; *** STACK USAGE *** 
  ; [rsp]   -> pointer to the logan struct

  mov   qword [rsp], 0

  ; malloc logan struct
  mov   rdi, LOGAN_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp], rax

  ; malloc logan format struct
  mov   rdi, LOGAN_FMT_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   qword [rax+LOGAN_FMT_OFF_COLORS], TRUE
  mov   qword [rax+LOGAN_FMT_OFF_TIMESTAMP], TRUE
  mov   qword [rax+LOGAN_FMT_OFF_TXT_MAX_LEN], LOGAN_DEFAULT_TXT_MAX_LEN

  mov   rdi, [rsp]
  mov   [rdi+LOGAN_OFF_FMT], rax
  mov   qword [rdi+LOGAN_OFF_OUT], STDOUT_FILENO
  mov   qword [rdi+LOGAN_OFF_LVL], LOG_TRACE  

  mov   rax, rdi

  jmp   .return

.error:
  mov   rdi, [rsp]
  test  rdi, rdi
  jz    .no_free

  call  free
  
.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; frees the logan struct and associated resources
; @param  rdi: pointer to the logan struct
; @return rax: return code
logan_free:
  sub   rsp, 0x8 

  ; *** STACK USAGE *** 
  ; [rsp]   -> pointer to the logan struct

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

  ; free logan format struct
  mov   rdi, [rsp]
  call  logan_get_fmt_struct
  cmp   rax, 0
  jle   .error

  mov   rdi, rax
  call  free
  cmp   rax, 0
  jl    .error    ; should be fatal here

  ; free struct
  mov   rdi, [rsp]
  call  free
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; returns the format struct of the logan struct
; @param  rdi: pointer to the logan struct
; @return rax: pointer to the logan format struct
logan_get_fmt_struct:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+LOGAN_OFF_FMT]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the format struct of the logan struct
; @param  rdi: pointer to the logan struct
; @param  rsi: pointer to the logan format struct
; @return rax: return code
logan_set_fmt_struct:
  cmp   rdi, 0
  jle   .error

  mov   [rdi+LOGAN_OFF_FMT], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the log prefix from the constant
; @param  rdi: log level constant
; @return rax: pointer to the log prefix
log_get_prefix:
  mov   rax, rdi
  mov   rbx, LOG_PREFIX_SIZE
  mul   rbx

  mov   rdi, log_prefixes
  add   rdi, rax

  cmp   rdi, log_prefixes_end
  jge   .error

  mov   rax, rdi

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; prints the error message and exits the programme with -1
; @param  rdi: pointer to the string
; @return rax: return code
log_fatal:
  ; rdi is already populated
  mov   rsi, LOG_FATAL
  call  _log

  ret

; prints the error message with a new line and exits the 
; programme with -1
; @param  rdi: pointer to the string
; @return rax: return code
log_fatalln:
  ; rdi is already populated
  mov   rsi, LOG_FATAL
  call  _logln

  ret

; prints the error message at the ERROR level
; @param  rdi: pointer to the string
; @return rax: return code
log_error:
  ; rdi is already populated
  mov   rsi, LOG_ERROR
  call  _log

  ret

; prints the error message at the ERROR level with a new line
; @param  rdi: pointer to the string
; @return rax: return code
log_errorln:
  ; rdi is already populated
  mov   rsi, LOG_ERROR
  call  _logln

  ret

; prints the error message at the WARN level
; @param  rdi: pointer to the string
; @return rax: return code
log_warn:
  ; rdi is already populated
  mov   rsi, LOG_WARN
  call  _log

  ret

; prints the error message at the WARN level with a new line
; @param  rdi: pointer to the string
; @return rax: return code
log_warnln:
  ; rdi is already populated
  mov   rsi, LOG_WARN
  call  _logln

  ret

; prints the error message at the INFO level
; @param  rdi: pointer to the string
; @return rax: return code
log_info:
  ; rdi is already populated
  mov   rsi, LOG_INFO
  call  _log

  ret

; prints the error message at the INFO level with a new line
; @param  rdi: pointer to the string
; @return rax: return code
log_infoln:
  ; rdi is already populated
  mov   rsi, LOG_INFO
  call  _logln

  ret

; prints the error message at the LOG level
; @param  rdi: pointer to the string
; @return rax: return code
log_log:
  ; rdi is already populated
  mov   rsi, LOG_LOG
  call  _log

  ret

; prints the error message at the LOG level with a new line
; @param  rdi: pointer to the string
; @return rax: return code
log_logln:
  ; rdi is already populated
  mov   rsi, LOG_LOG
  call  _logln

  ret

; prints the error message at the DEBUG level
; @param  rdi: pointer to the string
; @return rax: return code
log_debug:
  ; rdi is already populated
  mov   rsi, LOG_DEBUG
  call  _log

  ret

; prints the error message at the DEBUG level with a new line
; @param  rdi: pointer to the string
; @return rax: return code
log_debugln:
  ; rdi is already populated
  mov   rsi, LOG_DEBUG
  call  _logln

  ret

; prints the error message at the TRACE level
; @param  rdi: pointer to the string
; @return rax: return code
log_trace:
  ; rdi is already populated
  mov   rsi, LOG_TRACE
  call  _log

  ret

; prints the error message at the TRACE level with a new line
; @param  rdi: pointer to the string
; @return rax: return code
log_traceln:
  ; rdi is already populated
  mov   rsi, LOG_TRACE
  call  _logln

  ret

; logs the message at the level given as parameter
; fatal error also closes the programme with -1
; @param  rdi: pointer to the message
; @param  rsi: level
; @return rax: return code
_log:
  sub   rsp, 0x10

  ; *** STACK USAGE *** ;
  ; [rsp]     -> pointer to the message
  ; [rsp+0x8] -> log level constant

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  cmp   rdi, 0
  jle   .error

  mov   rdi, [rsp+0x8]
  call  log_get_prefix
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  print
  cmp   rax, 0
  jl    .error
  
  mov   rdi, [rsp]
  call  print
  
.error:
  cmp   qword [rsp+0x8], LOG_FATAL
  je    .exit

  mov   rax, FAILURE_CODE
  jmp   .return

.exit:
  mov   rdi, FAILURE_CODE
  call  exit

.return:
  add   rsp, 0x10
  ret

; logs the message at the level given as parameter with a new line
; fatal error also closes the programme with -1
; @param  rdi: pointer to the message
; @param  rsi: level
; @return rax: return code
_logln:
  sub   rsp, 0x10

  ; *** STACK USAGE *** ;
  ; [rsp]     -> pointer to the message
  ; [rsp+0x8] -> log level constant

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  cmp   rdi, 0
  jle   .error

  mov   rdi, [rsp+0x8]
  call  log_get_prefix
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  print
  cmp   rax, 0
  jl    .error
  
  mov   rdi, [rsp]
  call  println
  
.error:
  cmp   qword [rsp+0x8], LOG_FATAL
  je    .exit

  mov   rax, FAILURE_CODE
  jmp   .return

.exit:
  mov   rdi, FAILURE_CODE
  call  exit

.return:
  add   rsp, 0x10
  ret

; enables colors when logging with Logan
; @param  rdi: pointer to logan struct 
; @return rax: return code
logan_enable_colors:
  cmp   rdi, 0
  jle   .error

  ; get logan format
  mov   rax, [rdi+LOGAN_OFF_FMT]

  ; enable colors
  mov   [rax+LOGAN_FMT_OFF_COLORS], TRUE

  mov   rax, SUCCESS_CODE
  jmp   .return
  
.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; disables colors when logging with Logan
; @param  rdi: pointer to logan struct 
; @return rax: return code
logan_disable_colors:
  cmp   rdi, 0
  jle   .error

  ; get logan format
  mov   rax, [rdi+LOGAN_OFF_FMT]

  ; disable colors
  mov   [rax+LOGAN_FMT_OFF_COLORS], FALSE

  mov   rax, SUCCESS_CODE
  jmp   .return
  
.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; enables logging timestamp 
; timestamp are logged in UTC
; @param  rdi: pointer to logan struct 
; @return rax: return code
logan_enable_timestamp:
  cmp   rdi, 0
  jle   .error

  ; get logan format
  mov   rax, [rdi+LOGAN_OFF_FMT]

  ; enable timestamp
  mov   [rax+LOGAN_FMT_OFF_TIMESTAMP], TRUE

  mov   rax, SUCCESS_CODE
  jmp   .return
  
.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; disables logging timestamp 
; @param  rdi: pointer to logan struct 
; @return rax: return code
logan_disable_timestamp:
  cmp   rdi, 0
  jle   .error

  ; get logan format
  mov   rax, [rdi+LOGAN_OFF_FMT]

  ; disable timestamp
  mov   [rax+LOGAN_FMT_OFF_TIMESTAMP], FALSE

  mov   rax, SUCCESS_CODE
  jmp   .return
  
.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the output of the logs
; @param  rdi: pointer to logan struct 
; @param  rsi: file descriptor
; @return rax: return code
logan_set_output:
  cmp   rdi, 0
  jle   .error

  ; I think it is safe to assume that negative fd is an error
  cmp   rsi, 0
  jl    .error

  mov   [rdi+LOGAN_OFF_OUT], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return
  
.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the output file descriptor of the logs
; @param  rdi: pointer to logan struct 
; @return rax: file descriptor
logan_get_output:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+LOGAN_OFF_OUT]

  jmp   .return
  
.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the maximum length of the logs
; @param  rdi: pointer to logan struct 
; @param  rsi: max length
; @return rax: return code
logan_set_max_length:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  ; get logan format struct
  mov   rax, [rdi+LOGAN_OFF_FMT]

  mov   [rax+LOGAN_FMT_OFF_TXT_MAX_LEN], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return
  
.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the maximum length of the logs
; @param  rdi: pointer to logan struct 
; @return rax: max length
logan_get_max_length:
  cmp   rdi, 0
  jle   .error

  ; get logan format struct
  mov   rsi, [rdi+LOGAN_OFF_FMT]

  mov   rax, [rsi+LOGAN_FMT_OFF_TXT_MAX_LEN]

  jmp   .return
  
.error:
  mov   rax, FAILURE_CODE

.return:
  ret


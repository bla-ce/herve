section .data

LOG_FATAL equ 0
LOG_ERROR equ 1
LOG_WARN  equ 2
LOG_INFO  equ 3
LOG_LOG   equ 4
LOG_DEBUG equ 5
LOG_TRACE equ 6

; log levels prefixes
log_prefixes:
  fatal_prefix  db "[FATAL] ", NULL_CHAR
  error_prefix  db "[ERROR] ", NULL_CHAR
  warn_prefix   db "[WARN]  ", NULL_CHAR
  info_prefix   db "[INFO]  ", NULL_CHAR
  log_prefix    db "[LOG]   ", NULL_CHAR
  debug_prefix  db "[DEBUG] ", NULL_CHAR
  trace_prefix  db "[TRACE] ", NULL_CHAR
log_prefixes_end:

LOG_PREFIX_SIZE equ error_prefix - fatal_prefix

section .text

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



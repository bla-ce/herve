section .bss

logan_struct:
  logan_output  resq 1  ; file descriptor of the output
  logan_level   resq 1  ; minimum level to be printed
  logan_format  resb 1  ; pointer to the format struct
logan_struct_end:

section .data

LOGAN_STRUCT_LEN  equ logan_struct_end - logan_struct

; logan offsets
LOGAN_OFF_OUT equ 0x0
LOGAN_OFF_LVL equ LOGAN_OFF_OUT + 0x8
LOGAN_OFF_FMT equ LOGAN_OFF_LVL + 0x8

LOGAN_FMT_COLORS    equ 0001b
LOGAN_FMT_TIMESTAMP equ 0010b
LOGAN_FMT_PREFIXES  equ 0100b

; log levels constants
LOGAN_TRACE equ 0
LOGAN_DEBUG equ 1
LOGAN_LOG   equ 2
LOGAN_INFO  equ 3
LOGAN_WARN  equ 4
LOGAN_ERROR equ 5
LOGAN_FATAL equ 6

; log levels prefixes
logan_prefixes:
  trace_prefix  db "[TRACE] ", NULL_CHAR
  debug_prefix  db "[DEBUG] ", NULL_CHAR
  log_prefix    db "[LOG]   ", NULL_CHAR
  info_prefix   db "[INFO]  ", NULL_CHAR
  warn_prefix   db "[WARN]  ", NULL_CHAR
  error_prefix  db "[ERROR] ", NULL_CHAR
  fatal_prefix  db "[FATAL] ", NULL_CHAR
logan_prefixes_end:

logan_colors:
  trace_color  dq WHITE_FG
  debug_color  dq BRIGHT_BLUE_FG
  log_color    dq BLUE_FG
  info_color   dq CYAN_FG
  warn_color   dq YELLOW_FG
  error_color  dq BRIGHT_RED_FG
  fatal_color  dq RED_FG
logan_colors_end:

LOGAN_PREFIX_SIZE equ fatal_prefix - error_prefix

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

  mov   rdi, rax
  mov   qword [rdi+LOGAN_OFF_OUT], STDOUT_FILENO
  mov   qword [rdi+LOGAN_OFF_LVL], LOGAN_TRACE
  mov   qword [rdi+LOGAN_OFF_FMT], 0  ; disable everything

  ; enable colors
  or    qword [rdi+LOGAN_OFF_FMT], LOGAN_FMT_COLORS

  ; enable prefixes
  or    qword [rdi+LOGAN_OFF_FMT], LOGAN_FMT_PREFIXES

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

; frees the logan struct
; @param  rdi: pointer to the logan struct
; @return rax: return code
logan_free:
  sub   rsp, 0x8

  ; *** STACK USAGE ***
  ; [rsp]   -> pointer to the logan struct

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

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

  xor   rax, rax
  mov   al, byte [rdi+LOGAN_OFF_FMT]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the color from the constant
; @param  rdi: log level constant
; @return rax: pointer to the ansi color escape code
logan_get_color_from_level:
  cmp   rdi, 0
  jl    .default

  mov   rax, rdi
  mov   rbx, 8
  mul   rbx

  mov   rdi, logan_colors
  add   rdi, rax

  cmp   rdi, logan_colors_end
  jge   .default

  mov   rax, [rdi]

  jmp   .return

.default:
  mov   rax, DEFAULT_FG

.return:
  ret

; returns the log prefix from the constant
; @param  rdi: log level constant
; @return rax: pointer to the log prefix
logan_get_prefix:
  mov   rax, rdi
  mov   rbx, LOGAN_PREFIX_SIZE
  mul   rbx

  mov   rdi, logan_prefixes
  add   rdi, rax

  cmp   rdi, logan_prefixes_end
  jge   .error

  mov   rax, rdi

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; prints the message without prefixes
; @param  rdi: pointer to the logan struct
; @param  rsi: pointer to the message
; @return rax: return code
log:
  ; rdi and rsi are already populated
  mov   rdx, -1 ; disable prefix
  mov   rcx, FALSE
  call  _log

  ret

; prints the message without prefixes with a new line
; @param  rdi: pointer to the logan struct
; @param  rsi: pointer to the message
; @return rax: return code
logln:
  ; rdi and rsi are already populated
  mov   rdx, -1 ; disable prefix
  mov   rcx, TRUE
  call  _log

  ret

; prints the error message and exits the programme with -1
; @param  rdi: pointer to the logan struct
; @param  rsi: pointer to the message
; @return rax: return code
log_fatal:
  ; rdi and rsi are already populated
  mov   rdx, LOGAN_FATAL
  mov   rcx, FALSE
  call  _log

  ret

; prints the error message with a new line and exits the
; programme with -1
; @param  rdi: pointer to the logan struct
; @param  rsi: pointer to the string
; @return rax: return code
log_fatalln:
  ; rdi and rsi are already populated
  mov   rdx, LOGAN_FATAL
  mov   rcx, TRUE
  call  _logln

  ret

; prints the error message at the ERROR level
; @param  rdi: pointer to the logan struct
; @param  rsi: pointer to the message
; @return rax: return code
log_error:
  ; rdi and rsi are already populated
  mov   rdx, LOGAN_ERROR
  mov   rcx, FALSE
  call  _log

  ret

; prints the error message at the ERROR level with a new line
; @param  rdi: pointer to the logan struct
; @param  rsi: pointer to the struct
; @return rax: return code
log_errorln:
  ; rdi and rsi are already populated
  mov   rdx, LOGAN_ERROR
  mov   rcx, TRUE
  call  _logln

  ret

; prints the error message at the WARN level
; @param  rdi: pointer to the logan struct
; @param  rsi: pointer to the message
; @return rax: return code
log_warn:
  ; rdi and rsi are already populated
  mov   rdx, LOGAN_WARN
  mov   rcx, FALSE
  call  _log

  ret

; prints the error message at the WARN level with a new line
; @param  rdi: pointer to the logan struct
; @param  rsi: pointer to the struct
; @return rax: return code
log_warnln:
  ; rdi and rsi are already populated
  mov   rdx, LOGAN_WARN
  mov   rcx, TRUE
  call  _logln

  ret

; prints the message at the INFO level
; @param  rdi: pointer to the logan struct
; @param  rsi: pointer to the message
; @return rax: return code
log_info:
  ; rdi and rsi are already populated
  mov   rdx, LOGAN_INFO
  mov   rcx, FALSE
  call  _log

  ret

; prints the message at the INFO level with a new line
; @param  rdi: pointer to the logan struct
; @param  rsi: pointer to the struct
; @return rax: return code
log_infoln:
  ; rdi and rsi are already populated
  mov   rdx, LOGAN_INFO
  mov   rcx, TRUE
  call  _logln

  ret

; prints the message at the LOG level
; @param  rdi: pointer to the logan struct
; @param  rsi: pointer to the message
; @return rax: return code
log_log:
  ; rdi and rsi are already populated
  mov   rdx, LOGAN_LOG
  mov   rcx, FALSE
  call  _log

  ret

; prints the message at the LOG level with a new line
; @param  rdi: pointer to the logan struct
; @param  rsi: pointer to the struct
; @return rax: return code
log_logln:
  ; rdi and rsi are already populated
  mov   rdx, LOGAN_LOG
  mov   rcx, TRUE
  call  _logln

  ret

; prints the message at the DEBUG level
; @param  rdi: pointer to the logan struct
; @param  rsi: pointer to the message
; @return rax: return code
log_debug:
  ; rdi and rsi are already populated
  mov   rdx, LOGAN_DEBUG
  mov   rcx, FALSE
  call  _log

  ret

; prints the message at the DEBUG level with a new line
; @param  rdi: pointer to the logan struct
; @param  rsi: pointer to the struct
; @return rax: return code
log_debugln:
  ; rdi and rsi are already populated
  mov   rdx, LOGAN_DEBUG
  mov   rcx, TRUE
  call  _logln

  ret

; prints the message at the TRACE level
; @param  rdi: pointer to the logan struct
; @param  rsi: pointer to the message
; @return rax: return code
log_trace:
  ; rdi and rsi are already populated
  mov   rdx, LOGAN_TRACE
  mov   rcx, FALSE
  call  _log

  ret

; prints the message at the TRACE level with a new line
; @param  rdi: pointer to the logan struct
; @param  rsi: pointer to the struct
; @return rax: return code
log_traceln:
  ; rdi and rsi are already populated
  mov   rdx, LOGAN_TRACE
  mov   rcx, TRUE
  call  _logln

  ret

; logs the message at the level given as parameter
; fatal error also closes the programme with -1
; @param  rdi: pointer to the logan struct
; @param  rsi: pointer to the message
; @param  rdx: level
; @param  rcx: print new line bool
; @return rax: return code
_log:
  sub   rsp, 0x40
  sub   rsp, UNSIGNED_LONG_MAX_STR_SIZE

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the logan struct
  ; [rsp+0x8]   -> pointer to the message
  ; [rsp+0x10]  -> log level constant
  ; [rsp+0x18]  -> print new line flag
  ; [rsp+0x20]  -> file descriptor to be written to
  ; [rsp+0x28]  -> length of the message
  ; [rsp+0x30]  -> characters to print
  ; [rsp+0x38]  -> size of the timestamp string
  ; [rsp+0x40]  -> pointer to the stringified timestamp

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   [rsp+0x18], rcx

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; save the file descriptor
  mov   rdi, [rsp]
  call  logan_get_output
  cmp   rax, 0
  jl    .error

  mov   qword [rsp+0x20], rax

  cmp   qword [rsp+0x10], 0
  jl    .log_message  ; log without prefix

  mov   rdi, [rsp]
  call  logan_get_minimum_level
  cmp   rax, 0
  jl    .error

  cmp   [rsp+0x10], rax
  jl    .return

  ; check if prefixes are enabled
  mov   rdi, [rsp]
  xor   rax, rax
  mov   al, byte [rdi+LOGAN_OFF_FMT]

  and   al, LOGAN_FMT_PREFIXES
  cmp   al, 0
  je    .log_timestamp

  ; check if colors are enabled
  mov   rdi, [rsp]
  xor   rax, rax
  mov   al, byte [rdi+LOGAN_OFF_FMT]

  and   al, LOGAN_FMT_COLORS
  cmp   al, 0
  je    .no_colors

  mov   rdi, [rsp+0x10]
  call  logan_get_color_from_level
  cmp   rax, 0
  jl    .error

  mov   rdi, qword [rsp+0x20]
  mov   rsi, rax
  mov   rdx, CHANGE_COLOR_LEN
  call  write_file
  cmp   rax, 0
  jl    .error

.no_colors:
  mov   rdi, [rsp+0x10]
  call  logan_get_prefix
  cmp   rax, 0
  jl    .error

  mov   rdi, qword [rsp+0x20]
  mov   rsi, rax
  mov   rdx, LOGAN_PREFIX_SIZE
  call  write_file
  cmp   rax, 0
  jl    .error

  ; default color
  mov   rdi, qword [rsp+0x20]
  mov   rsi, DEFAULT_FG
  mov   rdx, CHANGE_COLOR_LEN
  call  write_file
  cmp   rax, 0
  jl    .error

.log_timestamp:
  ; check if timestamp are enabled
  mov   rdi, [rsp]
  xor   rax, rax
  mov   al, byte [rdi+LOGAN_OFF_FMT]

  and   al, LOGAN_FMT_TIMESTAMP
  cmp   al, 0
  je    .log_message

  ; get timestamp
  call  now
  cmp   rax, 0
  jle   .log_message

  ; stringify timestamp
  mov   rdi, rax
  lea   rsi, [rsp+0x40]
  mov   rdx, UNSIGNED_LONG_MAX_STR_SIZE
  call  to_string
  cmp   rax, 0
  jle   .log_message

  lea   rdi, [rsp+0x40]
  call  strlen
  cmp   rax, 0
  jle   .log_message

  ; print timestamp
  mov   rdi, qword [rsp+0x20]
  lea   rsi, [rsp+0x40]
  mov   rdx, rax
  call  write_file
  cmp   rax, 0
  jl    .error

  ; write space
  mov   qword [rsp+0x30], 0
  mov   byte [rsp+0x30], SPACE

  mov   rdi, qword [rsp+0x20]
  lea   rsi, [rsp+0x30]
  mov   rdx, 1
  call  write_file
  cmp   rax, 0
  jl    .error

.log_message:
  ; get message length
  mov   rdi, [rsp+0x8]
  call  strlen
  cmp   rax, 0
  jl    .error

  mov   qword [rsp+0x28], rax
  mov   rdi, qword [rsp+0x20]
  mov   rsi, [rsp+0x8]
  mov   rdx, qword [rsp+0x28]
  call  write_file
  cmp   rax, 0
  jl    .error

  cmp   qword [rsp+0x18], TRUE
  jne   .no_new_line

  ; write new line
  mov   qword [rsp+0x30], 0
  mov   byte [rsp+0x30], LINE_FEED

  mov   rdi, qword [rsp+0x20]
  lea   rsi, [rsp+0x30]
  mov   rdx, 1
  call  write_file
  cmp   rax, 0
  jl    .error

.no_new_line:
  cmp   qword [rsp+0x10], LOGAN_FATAL
  je    .exit

  jmp   .return

.error:
  cmp   qword [rsp+0x10], LOGAN_FATAL
  je    .exit

  mov   rax, FAILURE_CODE
  jmp   .return

.exit:
  mov   rdi, FAILURE_CODE
  call  exit

.return:
  add   rsp, UNSIGNED_LONG_MAX_STR_SIZE
  add   rsp, 0x40
  ret

; logs the message at the level given as parameter with a new line
; fatal error also closes the programme with -1
; @param  rdi: pointer to the logan struct
; @param  rsi: pointer to the message
; @param  rdx: level
; @return rax: return code
_logln:
  ; rdi, rsi and rcx are already populated
  mov   rcx, TRUE
  call  _log
  ret

; enables prefixes when logging with Logan
; @param  rdi: pointer to logan struct
; @return rax: return code
logan_enable_prefixes:
  cmp   rdi, 0
  jle   .error

  or    byte [rdi+LOGAN_OFF_FMT], LOGAN_FMT_PREFIXES

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; disables prefixes when logging with Logan
; @param  rdi: pointer to logan struct
; @return rax: return code
logan_disable_prefixes:
  cmp   rdi, 0
  jle   .error

  or    byte [rdi+LOGAN_OFF_FMT], LOGAN_FMT_PREFIXES
  xor   byte [rdi+LOGAN_OFF_FMT], LOGAN_FMT_PREFIXES

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; enables colors when logging with Logan
; @param  rdi: pointer to logan struct
; @return rax: return code
logan_enable_colors:
  cmp   rdi, 0
  jle   .error

  or    byte [rdi+LOGAN_OFF_FMT], LOGAN_FMT_COLORS

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

  or    byte [rdi+LOGAN_OFF_FMT], LOGAN_FMT_COLORS
  xor   byte [rdi+LOGAN_OFF_FMT], LOGAN_FMT_COLORS

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

  ; enable timestamp
  or    byte [rdi+LOGAN_OFF_FMT], LOGAN_FMT_TIMESTAMP

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

  xor   byte [rdi+LOGAN_OFF_FMT], LOGAN_FMT_TIMESTAMP

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


; sets the minimum level of the logs
; @param  rdi: pointer to logan struct
; @param  rsi: minimum level (included)
; @return rax: return code
logan_set_minimum_level:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   [rdi+LOGAN_OFF_LVL], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the minimum level of the logs
; @param  rdi: pointer to logan struct
; @return rax: minimum level
logan_get_minimum_level:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+LOGAN_OFF_LVL]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

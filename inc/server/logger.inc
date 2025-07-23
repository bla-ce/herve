; ***** IMPORTANT *****
; This logging implementation will be replaced soon to use 
; the generic library that is being implemented in /inc/log.
; This file will be used to integrate the new log library with 
; herve.
; ***** IMPORTANT *****

section .data
  logger_struct:
    output        dq STDOUT_FILENO          ; file descriptor  
    delimiter     dq log_default_delimiter  ; pointer to delimiter (string)
    mode          dq 0                      ; debug | release
    colors        db 1                      ; enable or disable color
  logger_struct_end:

  LOGGER_STRUCT_LEN equ logger_struct_end - logger_struct

  DEV_MODE  equ 0
  PROD_MODE equ 1

  DURATION_MAX_SIZE equ 0x10

  LOGGER_OFF_OUTPUT     equ 0x0
  LOGGER_OFF_DELIMITER  equ LOGGER_OFF_OUTPUT + 0x8
  LOGGER_OFF_MODE       equ LOGGER_OFF_DELIMITER + 0x8
  LOGGER_OFF_COLORS     equ LOGGER_OFF_MODE + 0x8

  log_default_delimiter db " | ", NULL_CHAR

  log_pre     db SPACE, "[LOG]", SPACE, NULL_CHAR
  _log_debug  db SPACE, "[DEBUG]", SPACE, NULL_CHAR
  error_pre   db SPACE, "[ERROR]", SPACE, NULL_CHAR

  ; ansi color
  RED_FG      db 0x1b, "[31m", NULL_CHAR
  GREEN_FG    db 0x1b, "[32m", NULL_CHAR
  YELLOW_FG   db 0x1b, "[33m", NULL_CHAR
  BLUE_FG     db 0x1b, "[34m", NULL_CHAR
  CYAN_FG     db 0x1b, "[36m", NULL_CHAR
  GRAY_FG     db 0x1b, "[90m", NULL_CHAR
  DEFAULT_FG  db 0x1b, "[39m", NULL_CHAR

  STR_PORT_MAX_LENGTH equ 6

  log_server_init_msg       db "Listening and serving HTTP on :", NULL_CHAR
  log_routes_available_msg  db "Routes available:", NULL_CHAR

section .text

; initialise a default logger
; @return rax: pointer to default logger
logger_default:
  sub   rsp, 0x8  

  mov   rdi, LOGGER_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp], rax

  mov   rdi, [rsp]
  mov   rsi, STDOUT_FILENO
  call  set_logger_output
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  lea   rsi, log_default_delimiter
  call  set_logger_delimiter
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  call  enable_color_output
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; returns the delimiter associated to the logger struct
; @param  rdi: pointer to the logger struct
; @return rax: logger mode
get_logger_mode:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+LOGGER_OFF_MODE]
  
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the mode associated to the logger struct
; @param  rdi: pointer to the logger struct
; @param  rsi: logger mode
; @return rax: return code
set_logger_mode:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   [rdi+LOGGER_OFF_MODE], rsi
  
  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the delimiter associated to the logger struct
; @param  rdi: pointer to the logger struct
; @return rax: pointer to the delimiter
get_logger_delimiter:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+LOGGER_OFF_DELIMITER]
  
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the delimiter associated to the logger struct
; @param  rdi: pointer to the logger struct
; @param  rsi: pointer to the delimiter
; @return rax: return code
set_logger_delimiter:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   [rdi+LOGGER_OFF_DELIMITER], rsi
  
  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; disables color to be outputted
; @param: rdi: pointer to the logger struct
; @return rax: return code
disable_color_output:
  cmp   rdi, 0
  jle   .error

  mov   byte [rdi+LOGGER_OFF_COLORS], 0

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; enables color to be outputted
; @param: rdi: pointer to the logger struct
; @return rax: return code
enable_color_output:
  cmp   rdi, 0
  jle   .error

  mov   byte [rdi+LOGGER_OFF_COLORS], 1

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns whether colors can be outputted
; @param  rdi: pointer to the logger struct
; @return rax: bool
can_color_be_ouputted:
  cmp   rdi, 0
  jle   .error

  mov   al, byte [rdi+LOGGER_OFF_COLORS]
  movzx rax, al

  jmp   .return
  
.error:
  mov   rax, FALSE

.return:
  ret

; returns the file descriptor where logs are written
; @param  rdi: pointer to the logger struct
; @return rax: file descriptor
get_logger_output:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+LOGGER_OFF_OUTPUT]
  
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the file descriptor where logs are written
; @param  rdi: pointer to the logger struct
; @param  rsi: file descriptor
; @return rax: return code
set_logger_output:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   [rdi+LOGGER_OFF_OUTPUT], rsi
  
  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; writes a string to the log output.
; @param  rdi: pointer to the logger struct
; @param  rsi: pointer to the string
log:
  sub   rsp, 0x18

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  mov   rdi, [rsp+0x8]
  call  strlen
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; get logger output
  mov   rdi, [rsp]
  call  get_logger_output
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, [rsp+0x8]
  mov   rdx, [rsp+0x10]
  call  write_file
  cmp   rax, 0
  jl    .error

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; writes a string to the log output followed by a line feed.
; @param  rdi: pointer to the logger struct
; @param  rsi: pointer to the string
logln:
  sub   rsp, 0x10

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  call  log 
  cmp   rax, 0
  jl    .return

  mov   rdi, [rsp]
  mov   rsi, LINE_FEED
  call  log_char

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; writes a single character to the log output.
; @param  rdi: pointer to logger struct
; @param  rsi: character to print
log_char:
  sub   rsp, 0x10

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   [rsp], rdi
  mov   qword [rsp+0x8], rsi

  ; get logger output
  mov   rdi, [rsp]
  call  get_logger_output
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rax, SYS_WRITE
  lea   rsi, [rsp+0x8]
  mov   rdx, 1
  syscall

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; @param  rdi: pointer to logger struct
; @return rax: return code
log_delimiter:
  sub   rsp, 0x8

  cmp   rdi, 0
  jle   .error

  mov   [rsp], rdi
  call  get_logger_delimiter

  mov   rdi, [rsp]
  mov   rsi, rax
  call  log
  
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; log current date and time
; @param  rdi: pointer to the logger struct
; @param  rsi: (optional) timestamp, if set to 0 get now
; @return rax: exit code
log_time:
  sub   rsp, 0x10
  sub   rsp, DT_STRUCT_LEN
  sub   rsp, DEFAULT_TIME_STR_LEN

  ; *** STACK USAGE *** ;
  ; [rsp]                     -> pointer to logger struct
  ; [rsp+0x8]                 -> timestamp
  ; [rsp+0x10]                -> pointer to dt struct
  ; [rsp+0x10+DT_STRUCT_LEN]  -> log string

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  je    .get_unix
  jl    .error

  mov   rdi, rsi

  jmp   .print_time

.get_unix:
  call  now
  cmp   rax, 0
  jl    .error

  mov   rdi, rax

.print_time:
  mov   [rsp+0x8], rdi

  ; malloc dt struct
  mov   rdi, [rsp+0x8]
  lea   rsi, [rsp+0x10]
  call  unix_to_date_time
  cmp   rax, 0
  jl    .error

  lea   rdi, [rsp+0x10]
  lea   rsi, [rsp+0x10+DT_STRUCT_LEN]
  call  time_to_str
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  lea   rsi, [rsp+0x10+DT_STRUCT_LEN]
  call  log
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return
  
.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, DT_STRUCT_LEN
  add   rsp, DEFAULT_TIME_STR_LEN
  add   rsp, 0x10
  ret


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

  LOGGER_OFF_OUTPUT     equ 0x0
  LOGGER_OFF_DELIMITER  equ LOGGER_OFF_OUTPUT + 0x8
  LOGGER_OFF_MODE       equ LOGGER_OFF_DELIMITER + 0x8
  LOGGER_OFF_COLORS     equ LOGGER_OFF_MODE + 0x8

  log_default_delimiter db " | ", NULL_CHAR

  log_pre   db SPACE, "[LOG]", SPACE, NULL_CHAR
  log_debug db SPACE, "[DEBUG]", SPACE, NULL_CHAR
  error_pre db SPACE, "[ERROR]", SPACE, NULL_CHAR

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

; returns the foreground color of the status code
; @param  rdi: status code (int)
; @param  rax: pointer to the ansi sequence
get_status_code_color:
  cmp   rdi, 100
  jl    .error

  cmp   rdi, 599
  jg    .error

  cmp   rdi, 500
  jge   .red

  cmp   rdi, 400
  jge   .yellow

  cmp   rdi, 300
  jge   .cyan

  cmp   rdi, 200
  jge   .green

  cmp   rdi, 100
  jge   .blue

.red:
  lea   rax, [RED_FG]
  jmp   .return

.yellow:
  lea   rax, [YELLOW_FG]
  jmp   .return

.cyan:
  lea   rax, [CYAN_FG]
  jmp   .return

.green:
  lea   rax, [GREEN_FG]
  jmp   .return

.blue:
  lea   rax, [BLUE_FG]
  jmp   .return
  
.error:
  lea   rax, [DEFAULT_FG]

.return:
  ret

; logs the status code
; @param  rdi: pointer to the logger struct
; @param  rsi: status code (int)
log_status_code:
  sub   rsp, 0x18 

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 100
  jl    .error

  cmp   rsi, 599
  jg    .error

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  ; can colors be outputted?
  mov   rdi, [rsp]
  call  can_color_be_ouputted
  test  rax, rax
  jz    .no_color

  mov   rdi, [rsp+0x8]
  call  get_status_code_color
  cmp   rax, 0
  jl    .error

  ; print ansi to change color
  mov   rdi, [rsp]
  mov   rsi, rax
  call  log
  
.no_color:
  mov   rdi, 4
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; print status code
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x10]
  mov   rdx, 4 ; add one for null char
  call  to_string

  mov   rdi, [rsp]
  mov   rsi, rax
  call  log

  mov   rdi, [rsp+0x10]
  call  free

  ; back to default color
  mov   rdi, [rsp]
  lea   rsi, [DEFAULT_FG]
  call  log

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, [rsp+0x10]
  test  rax, rax
  jz    .no_free

  mov   rdi, rax
  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
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

; initial log of the available routes and port the server is listening to
; @param  rdi: pointer to server struct
log_server_init:
  sub   rsp, 0x20

  mov   [rsp], rdi
  mov   qword [rsp+0x10], 0

  ; get logger output
  mov   rdi, [rsp]
  call  get_server_logger
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; log the logo :)
  mov   rdi, [rsp+0x8]
  lea   rsi, [HERVE_LOGO]
  call  logln

  mov   rdi, [rsp+0x8]
  lea   rsi, [log_debug]
  call  log

  mov   rdi, [rsp+0x8]
  lea   rsi, [log_server_init_msg]
  call	log 

  mov   rdi, STR_PORT_MAX_LENGTH
  call  malloc

  cmp   rax, 0
  jl    .return

  mov   [rsp+0x10], rax

  mov   rsi, [rsp]
  mov   rdi, qword [rsi+SERVER_OFF_PORT]
  mov   rsi, rax
  mov   rdx, STR_PORT_MAX_LENGTH
  call  to_string

  cmp   rax, 0
  jl    .return

  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call	logln

  ; free port string
  mov   rdi, [rsp+0x10]
  call  free

  mov   qword [rsp+0x10], 0

  mov   rdi, [rsp+0x8]
  mov   rsi, LINE_FEED
  call  log_char

  mov   rdi, [rsp+0x8]
  lea   rsi, [log_debug]
  call  log

  mov   rdi, [rsp+0x8]
  lea   rsi, [log_routes_available_msg]
  call	logln

  ; load server routes
  mov   rdi, [rsp]
  call  get_server_routes

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  cmp   rax, 0
  je    .end_loop

.loop:
  ; log debug 
  mov   rdi, [rsp+0x8]
  lea   rsi, [log_debug]
  call	log

  ; load routes_list
  mov   rdi, [rsp+0x18]

  ; log method
  ; go to method
  mov   rsi, [rdi+ROUTE_OFF_METHOD]
  mov   rdi, [rsp+0x8]
  call  log

  ; print space
  mov   rdi, [rsp+0x8]
  mov   rsi, SPACE
  call  log_char

  ; load url and log it
  mov   rdi, [rsp+0x8]
  mov   rax, [rsp+0x18]
  mov   rsi, [rax+ROUTE_OFF_URL]
  call  logln

  mov   rsi, [rsp+0x18]
  cmp   qword [rsi+ROUTE_OFF_NEXT], 0
  je    .end_loop

  add   rsi, ROUTE_OFF_NEXT
  mov   rdi, [rsi]

  mov   [rsp+0x18], rdi
  
  jmp   .loop
.end_loop:

  ; log new line
  mov   rdi, [rsp+0x8]
  mov   rsi, LINE_FEED
  call	log_char

  jmp   .return

.error:
  ; free port string
  mov   rax, qword [rsp+0x10]
  test  rax, rax
  jz    .no_free

  mov   rdi, rax
  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
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

; log the duration of the context
; @param  rdi: pointer to the logger struct
; @param  rsi: pointer to the context struct
; @return rax: return code
log_ctx_duration:
  sub   rsp, 0x28

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

  mov   rdi, 0x18 ; TODO: Update magic value
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  mov   rsi, rax
  add   rsi, 0x18 ; TODO: Update magic value

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
  call  log

  mov   rdi, [rsp+0x20]
  call  free

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, qword [rsp+0x20]
  test  rax, rax
  jz    .no_free

  mov   rdi, rax
  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x28
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

; log current date and time
; @param  rdi: pointer to the logger struct
; @param  rsi: (optional) timestamp, if set to 0 get now
; @return rax: exit code
log_time:
  sub   rsp, 0x10 ; store timestamp

  cmp   rdi, 0
  jle   .error

  mov   [rsp], rdi

  cmp   rsi, 0
  je    .get_unix
  jl    .error

  jmp   .print_time

.get_unix:
  call  now
  cmp   rax, 0
  jl    .error

  mov   rdi, rax

.print_time:
  call  unix_to_date_time

  mov   rdi, 5  ; add one for null char
  call  malloc
  
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, qword [dt_year]
  mov   rsi, rax
  mov   rdx, 5
  call  to_string

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, rax
  call  log

  mov   rdi, [rsp+0x8]
  call  free

  mov   rdi, [rsp]
  mov   rsi, SLASH
  call  log_char

  cmp   qword [dt_month], 10
  jge   .no_zero_month

  mov   rdi, [rsp]
  mov   rsi, ZERO_CHAR
  call  log_char

.no_zero_month:
  mov   rdi, 3
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, qword [dt_month]
  mov   rsi, rax
  mov   rdx, 3
  call  to_string

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, rax
  call  log

  mov   rdi, [rsp]
  mov   rsi, SLASH
  call  log_char

  cmp   qword [dt_day], 10
  jge   .no_zero_day

  mov   rdi, [rsp]
  mov   rsi, ZERO_CHAR
  call  log_char

.no_zero_day:
  mov   rdi, qword [dt_day]
  mov   rsi, [rsp+0x8]
  mov   rdx, 3
  call  to_string

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, rax
  call  log

  mov   rdi, [rsp]
  mov   rsi, SPACE
  call	log_char

  mov   rdi, [rsp]
  mov   rsi, HYPHEN
  call	log_char

  mov   rdi, [rsp]
  mov   rsi, SPACE
  call	log_char

  cmp   qword [dt_hour], 10
  jge   .no_zero_hour

  mov   rdi, [rsp]
  mov   rsi, ZERO_CHAR
  call	log_char

.no_zero_hour:
  mov   rdi, qword [dt_hour]
  mov   rsi, [rsp+0x8]
  mov   rdx, 3
  call  to_string

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, rax
  call  log

  mov   rdi, [rsp]
  mov   rsi, COLON
  call  log_char

  cmp   qword [dt_minute], 10
  jge   .no_zero_minute

  mov   rdi, [rsp]
  mov   rsi, ZERO_CHAR
  call  log_char

.no_zero_minute:
  mov   rdi, qword [dt_minute]
  mov   rsi, [rsp+0x8]
  mov   rdx, 3
  call  to_string

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, rax
  call  log

  mov   rdi, [rsp]
  mov   rsi, COLON
  call  log_char

  cmp   qword [dt_second], 10
  jge   .no_zero_second

  mov   rdi, [rsp]
  mov   rsi, ZERO_CHAR
  call  log_char

.no_zero_second:
  mov   rdi, qword [dt_second]
  mov   rsi, [rsp+0x8]
  mov   rdx, 3
  call  to_string

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, rax
  call  log

  mov   rdi, [rsp+0x8]
  call  free

  mov   rdi, [rsp]
  mov   rsi, SPACE
  call  log_char

  mov   rdi, [rsp]
  lea   rsi, [utc_str]
  call  log

  mov   rax, SUCCESS_CODE
  jmp   .return
  
.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret



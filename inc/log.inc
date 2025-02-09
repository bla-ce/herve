section .data
STR_PORT_MAX_LENGTH equ 0x6 ; add one for null_char

log_server_init_msg       db "Listening and serving HTTP on :", NULL_CHAR
log_routes_available_msg  db "Routes available:", NULL_CHAR

log_pre   db SPACE, "[LOG]", SPACE, NULL_CHAR
log_debug db SPACE, "[DEBUG]", SPACE, NULL_CHAR
error_pre db SPACE, "[ERROR]", SPACE, NULL_CHAR

log_delimiter db SPACE, VERTICAL_BAR, SPACE, NULL_CHAR
log_arrow     db SPACE, "->", SPACE, NULL_CHAR

; ansi color
red_fg      db 0x1b, "[31m", NULL_CHAR
green_fg    db 0x1b, "[32m", NULL_CHAR
yellow_fg   db 0x1b, "[33m", NULL_CHAR
default_fg  db 0x1b, "[39m", NULL_CHAR

; bool to enable/disable color
log_color dq 1

; where to store logs, standard output by default 
log_output dq STDOUT

section .text

; TODO: merge log and print functions
; writes a string to the log output.
; @param  rdi: string
; @param  rsi: (optional) string length
log:
  sub   rsp, 0x8

  cmp   rdi, 0
  jle   .error

  mov   rax, rsi
  cmp   rax, 0
  jg    .known_length

  mov   [rsp], rdi

  call  strlen
  test  rax, rax
  jz    .return

  mov   rdi, [rsp]

.known_length:
  xor   rsi, rsi
  lea   rsi, [rdi]
  mov   rdx, rax
  mov   rax, SYS_WRITE
  mov   rdi, qword [log_output]
  syscall

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; writes a string to the log output followed by a line feed.
; @param  rdi: string
; @param  rsi: (optional) string length
logln:
  sub   rsp, 0x8

  cmp   rdi, 0
  jle   .error

  call  log 
  cmp   rax, 0
  jl    .return

  mov   qword [rsp], LINE_FEED

  xor   rsi, rsi

  mov   rax, SYS_WRITE
  mov   rdi, qword [log_output]
  lea   rsi, [rsp]
  mov   rdx, 1
  syscall

  ; clean stack
  xor   rax, rax

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; writes a single character to the log output.
; @param  rdi: character to print
log_char:
  sub   rsp, 0x8

  mov   qword [rsp], rdi

  xor   rsi, rsi

  mov   rax, SYS_WRITE
  mov   rdi, qword [log_output]
  lea   rsi, [rsp]
  mov   rdx, 1
  syscall

  add   rsp, 0x8
  ret

; sets the log output file descriptor.
; TODO: accept file path and do everything inside
; @param  rdi: fd
set_log_output:
  mov   qword [log_output], rdi
  ret

; Disables colored log output by setting log_color to 0
disable_log_color:
  mov   qword [log_color], 0
  ret

; Enables colored log output by setting log_color to 1
enable_log_color:
  mov   qword [log_color], 1
  ret

; initial log of the available routes and port the server is listening to
; @param  rdi: pointer to server struct
log_server_init:
  sub   rsp, 0x20

  mov   [rsp], rdi

  xor   rdi, rdi
  lea   rdi, [log_debug]
  mov   rsi, 0
  call  print

  xor   rdi, rdi
  lea   rdi, [log_server_init_msg]
  mov   rsi, 0
  call	print 

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

  mov   rdi, rax
  mov   rsi, 0
  call	println

  ; free port string
  mov   rdi, [rsp+0x10]
  call  free

  mov   rdi, LINE_FEED
  call  print_char

  xor   rdi, rdi
  lea   rdi, [log_debug]
  mov   rsi, 0
  call  print

  xor   rdi, rdi
  lea   rdi, [log_routes_available_msg]
  mov   rsi, 0
  call	println

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
  xor   rdi, rdi
  lea   rdi, [log_debug]
  mov   rsi, 0
  call	print

  ; load routes_list
  mov   rsi, [rsp+0x18]

  ; log method
  ; go to method
  mov   rdi, [rsi+ROUTE_OFF_METHOD]
  mov   rsi, 0
  call  print

  ; print space
  mov   rdi, SPACE
  call  print_char

  ; load url and log it
  mov   rsi, [rsp+0x18]
  mov   rdi, [rsi+ROUTE_OFF_URL]
  mov   rsi, 0
  call  println

  mov   rsi, [rsp+0x18]
  cmp   qword [rsi+ROUTE_OFF_NEXT], 0
  je    .end_loop

  add   rsi, ROUTE_OFF_NEXT
  mov   rdi, [rsi]

  mov   [rsp+0x18], rdi
  
  jmp   .loop
.end_loop:

  ; log new line
  mov   rdi, LINE_FEED
  call	print_char

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret

; store duration inside log_duration
; TODO: pass duration as arg
log_get_duration:
  sub   rsp, 0x10
  mov   qword [rsp], 0 ; length of duration str
  mov   qword [rsp+0x8], 0

  mov   rdi, 0x18 ; TODO: Update magic value
  mov   rsi, 1
  call  calloc

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rsi, rax
  add   rsi, 0x18 ; TODO: Update magic value

  ; print ns backwards
  mov   byte [rsi], LOWERCASE_S
  dec   rsi
  mov   byte [rsi], LOWERCASE_N
  dec   rsi

  mov   rax, qword [duration]

  ; convert ns in ms if duration is greater than 999ns
  cmp   rax, 999
  jl    .loop

  xor   rdx, rdx
  mov   rcx, 1000
  div   rcx

  ; Replace 'n' by 'μ' (UTF-8 encoding) 
  inc   rsi
  mov   byte [rsi], MICRO_FIRST     ; First byte of the UTF-8 sequence for 'μ'
  inc   rsi
  mov   byte [rsi], MICRO_SECOND    ; Second byte of the UTF-8 sequence for 'μ'
  inc   rsi
  mov   byte [rsi], LOWERCASE_S
  sub   rsi, 3    ; go back to original position
  
.loop:
  xor   rdx, rdx
  mov   rcx, 0xA  ; divisor, dividend in rax
  div   rcx       ; quotient in rax and remainder in edx

  add   rdx, ZERO_CHAR

  mov   byte [rsi], dl

  dec   rsi

  cmp   rax, 0
  jne   .loop

  ; print duration
  mov   rdi, [rsp+0x8]
  mov   rsi, 0x18 ; TODO: Update magic value
  add   rsi, 2
  call  log

  jmp   .free

.error:
  cmp   qword [rsp+0x8], 0
  jne   .return

.free:
  mov   rdi, [rsp+0x8]
  call  free

.return:
  add   rsp, 0x10
  ret

; log the current request
; @param  rdi: request
log_request:
  sub   rsp, 0x8

  mov   [rsp], rdi

  ; [LOG] 
  xor   rdi, rdi
  lea   rdi, [log_pre]
  mov   rsi, 0
  call  log

  ; log time
  mov   rdi, qword [start_tv_sec]
  call  log_time

  xor   rdi, rdi
  lea   rdi, [log_delimiter]
  mov   rsi, 0
  call  log

  call  log_get_duration

  xor   rdi, rdi
  lea   rdi, [log_delimiter]
  mov   rsi, 0
  call  log

  ; log status code
  call  log_status_code

  ; log delimiter
  xor   rdi, rdi
  lea   rdi, [log_delimiter]
  mov   rsi, 0
  call  log

  ; log method
  mov   rsi, [rsp]
  add   rsi, REQ_OFF_METHOD
  mov   rdi, [rsi]
  mov   rsi, 0
  call  log

  xor   rdi, rdi
  lea   rdi, [log_arrow]
  mov   rsi, 0
  call  log

  ; log url
  mov   rsi, [rsp]
  add   rsi, REQ_OFF_URL
  mov   rdi, [rsi]
  mov   rsi, 0
  call  logln

  mov   rax, 0

  add   rsp, 0x8
  ret

; log the status code
; TODO: pass status code as argument
log_status_code:
  sub   rsp, 0x8

  mov   rdi, 204 ; TODO: CHANGE to read real status code
  cmp   qword [log_color], 0
  je    .no_color

  cmp   rdi, BAD_REQUEST
  jl    .green

  cmp   rdi, INTERNAL_SERVER_ERROR
  jl    .yellow

  jmp   .red

  ; load green
.green:
  lea   rdi, [green_fg]
  jmp   .print_color

.yellow:
  lea   rdi, [yellow_fg]
  jmp   .print_color

.red:
  lea   rdi, [red_fg]

.print_color:
  ; print ansi to change color
  mov   rsi, 0
  call  log

.no_color:
  mov   rdi, 4
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   [rsp], rax

  ; print status code
  mov   rdi, 204 ; TODO: CHANGE after testing
  mov   rsi, rax
  mov   rdx, 4 ; add one for null char
  call  to_string

  mov   rdi, rax
  mov   rsi, 0
  call  log

  mov   rdi, [rsp]
  call  free

  ; back to default color
  lea   rdi, [default_fg]
  mov   rsi, 0
  call  log

.error:
  add   rsp, 0x8
  ret

; log current date and time
; @param  rdi: (optional) timestamp, if set to 0 get now
; @return rax: exit code
log_time:
  sub   rsp, 0x10 ; store timestamp

  cmp   rdi, 0
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

  mov   rdi, rax
  mov   rsi, 0
  call  log

  mov   rdi, [rsp+0x8]
  call  free

  mov   rdi, SLASH
  call  log_char

  cmp   qword [dt_month], 10
  jge   .no_zero_month

  mov   rdi, ZERO_CHAR
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

  mov   rdi, rax
  mov   rsi, 0
  call  log

  mov   rdi, SLASH
  call  log_char

  cmp   qword [dt_day], 10
  jge   .no_zero_day

  mov   rdi, ZERO_CHAR
  call  log_char

.no_zero_day:
  mov   rdi, qword [dt_day]
  mov   rsi, [rsp+0x8]
  mov   rdx, 3
  call  to_string

  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, 0
  call  log

  mov   rdi, SPACE
  call	log_char

  mov   rdi, HYPHEN
  call	log_char

  mov   rdi, SPACE
  call	log_char

  cmp   qword [dt_hour], 10
  jge   .no_zero_hour

  mov   rdi, ZERO_CHAR
  call	log_char

.no_zero_hour:
  mov   rdi, qword [dt_hour]
  mov   rsi, [rsp+0x8]
  mov   rdx, 3
  call  to_string

  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, 0
  call  log

  mov   rdi, COLON
  call  log_char

  cmp   qword [dt_minute], 10
  jge   .no_zero_minute

  mov   rdi, ZERO_CHAR
  call  log_char

.no_zero_minute:
  mov   rdi, qword [dt_minute]
  mov   rsi, [rsp+0x8]
  mov   rdx, 3
  call  to_string

  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, 0
  call  log

  mov   rdi, COLON
  call  log_char

  cmp   qword [dt_second], 10
  jge   .no_zero_second

  mov   rdi, ZERO_CHAR
  call  log_char

.no_zero_second:
  mov   rdi, qword [dt_second]
  mov   rsi, [rsp+0x8]
  mov   rdx, 3
  call  to_string

  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, 0
  call  log

  mov   rdi, [rsp+0x8]
  call  free

  mov   rdi, SPACE
  call  log_char

  xor   rdi, rdi
  lea   rdi, [utc_str]
  mov   rsi, 0
  call  log

  mov   rax, SUCCESS_CODE
  jmp   .return
  
.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret


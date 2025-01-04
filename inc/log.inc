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
  push  rbp
  mov   rbp, rsp

  mov   rax, rsi
  cmp   rax, 0
  jg    .known_length

  push  rdi
  call  strlen
  test  rax, rax
  jz    .return

  pop   rdi

.known_length:
  lea   rsi, [rdi]
  mov   rdx, rax
  mov   rax, SYS_WRITE
  mov   rdi, qword [log_output]
  syscall

.return:
  leave
  ret

; writes a string to the log output followed by a line feed.
; @param  rdi: string
; @param  rsi: (optional) string length
logln:
  push  rbp
  mov   rbp, rsp

  call  log 
  cmp   rax, 0
  jl    .return

  push  LINE_FEED

  mov   rax, SYS_WRITE
  mov   rdi, qword [log_output]
  lea   rsi, [rsp]
  mov   rdx, 1
  syscall

  ; clean stack
  pop   rax
  xor   rax, rax

.return:
  leave  
  ret

; writes a single character to the log output.
; @param  rdi: character to print
log_char:
  push  rbp
  mov   rbp, rsp

  push  rdi

  mov   rax, SYS_WRITE
  mov   rdi, qword [log_output]
  lea   rsi, [rsp]
  mov   rdx, 1
  syscall

  leave
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
log_server_init:
  sub   rsp, 0x18 ; routes count for route logs

  lea   rdi, [log_debug]
  mov   rsi, 0
  call  print

  lea   rdi, [log_server_init_msg]
  mov   rsi, 0
  call	print 

  mov   rdi, STR_PORT_MAX_LENGTH
  call  malloc

  cmp   rax, 0
  jl    .return

  mov   [rsp+0x10], rax

  mov   rdi, qword [port]
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

  lea   rdi, [log_debug]
  mov   rsi, 0
  call  print

  lea   rdi, [log_routes_available_msg]
  mov   rsi, 0
  call	println

  ; log routes - [DEBUG] METHOD ROUTE
  ; keep track of routes
  mov   qword [rsp], 0

.loop:
  ; log debug 
  lea   rdi, [log_debug]
  mov   rsi, 0
  call	print

  ; load routes_list
  lea   rdi, [routes_list]

  ; get route pos
  xor   rdx, rdx
  mov   rax, qword [rsp]
  mov   rbx, STRUCT_ROUTE_LEN
  mul   rbx

  ; go to route
  add   rdi, rax

  ; save route pos onto the stack
  mov   [rsp+0x8], rdi

  ; log method
  ; go to method
  add   rdi, URL_MAX_LEN
  mov   rsi, METHOD_MAX_LEN
  call  print

  ; print space
  mov   rdi, SPACE
  call  print_char

  ; load url and log it
  mov   rdi, [rsp+0x8]
  mov   rsi, URL_MAX_LEN
  call  println

  inc   qword [rsp]
  
  mov   rax, qword [rsp]
  cmp   rax, qword [routes_count]
  je    .return
  
  jmp   .loop

.return:
  ; log new line
  mov   rdi, LINE_FEED
  call	print_char

  add   rsp, 0x18
  ret

; store duration inside log_duration
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
  mov   byte [rsi], LOW_S_CHAR
  dec   rsi
  mov   byte [rsi], LOW_N_CHAR
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
  mov   byte [rsi], LOW_S_CHAR
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
log_request:
  ; [LOG] 
  lea   rdi, [log_pre]
  mov   rsi, 0
  call  log

  ; log time
  mov   rdi, qword [start_tv_sec]
  call  log_time

  lea   rdi, [log_delimiter]
  mov   rsi, 0
  call  log

  call  log_get_duration

  lea   rdi, [log_delimiter]
  mov   rsi, 0
  call  log

  ; log status code
  call  log_status_code

  ; log delimiter
  lea   rdi, [log_delimiter]
  mov   rsi, 0
  call  log

  ; log method
  lea   rdi, [request_method]
  mov   rsi, METHOD_MAX_LEN
  call  log

  lea   rdi, [log_arrow]
  mov   rsi, 0
  call  log

  ; log url
  lea   rdi, [request_url]
  mov   rsi, 0
  call  logln

  mov   rax, 0
  ret

; log the status code
log_status_code:
  sub   rsp, 0x8

  mov   rdi, qword [response_status_code]
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
  mov   rdi, qword [response_status_code]
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
  je   .get_unix

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


section .data
log_server_init_msg       db "Listening and serving HTTP on :", NULL_CHAR
log_routes_available_msg  db "Routes available:", NULL_CHAR

log_pre   db SPACE, "[LOG]", SPACE, NULL_CHAR
log_debug db SPACE, "[DEBUG]", SPACE, NULL_CHAR
error_pre db SPACE, "[ERROR]", SPACE, NULL_CHAR

log_delimiter db SPACE, VERTICAL_BAR, SPACE, NULL_CHAR
log_arrow     db SPACE, "->", SPACE, NULL_CHAR

log_duration  times LONG_SIZE+2   db SPACE

; ansi color
red_fg      db 0x1b, "[31m", NULL_CHAR
green_fg    db 0x1b, "[32m", NULL_CHAR
yellow_fg   db 0x1b, "[33m", NULL_CHAR
default_fg  db 0x1b, "[39m", NULL_CHAR

section .text
; initial log of the available routes and port the server is listening to
log_server_init:
  sub   rsp, 0x10 ; routes count for route logs

  lea   rdi, [log_debug]
  mov   rsi, 0
  call  print

  lea   rdi, [log_server_init_msg]
  mov   rsi, 0
  call	print 

  mov   rdi, qword [port]
  call  to_string

  lea   rdi, [rax]
  mov   rsi, 0
  call	println

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

  mov   rax, SUCCESS_CODE
  add   rsp, 0x10
  ret

; store duration inside log_duration
log_get_duration:
  sub   rsp, 0x8 ; duration_str length
  mov   qword [rsp], 0

  lea   rsi, [log_duration]
  add   rsi, LONG_SIZE

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
  sub   rsi, 3                ; go back to original position
  
.loop:
  xor   rdx, rdx
  mov   rcx, 0xA              ; divisor, dividend in rax
  div   rcx                   ; quotient in rax and remainder in edx

  add   rdx, ZERO_CHAR

  mov   byte [rsi], dl

  dec   rsi

  cmp   rax, 0
  jne   .loop

  add   rsp, 0x8
  mov   rax, SUCCESS_CODE
  ret

log_request:
  ; clear log duration and time
  mov   rcx, LONG_SIZE
  add   rcx, 2
  mov   rax, SPACE
  lea   rdi, [log_duration]
  rep   stosb

  ; [LOG] 
  lea   rdi, [log_pre]
  mov   rsi, 0
  call  print

  ; log time
  mov   rdi, qword [end_tv_sec]
  call  log_time

  lea   rdi, [log_delimiter]
  mov   rsi, 0
  call  print

  call  log_get_duration

  ; log request duration
  lea   rdi, [log_duration]
  mov   rsi, LONG_SIZE
  add   rsi, 2
  call  print

  lea   rdi, [log_delimiter]
  mov   rsi, 0
  call  print

  ; log status code
  call  log_status_code


  ; log delimiter
  lea   rdi, [log_delimiter]
  mov   rsi, 0
  call  print

  ; log method
  lea   rdi, [request_method]
  mov   rsi, METHOD_MAX_LEN
  call  print

  lea   rdi, [log_arrow]
  mov   rsi, 0
  call  print

  ; log url
  lea   rdi, [request_url]
  mov   rsi, URL_MAX_LEN
  call  println

  mov   rax, 0
  ret

log_malformed_request:
  mov   rsi, 0
  call  println

  mov   rax, 0
  ret

; log the status code with corresponding color
log_status_code:
  mov   rdi, qword [response_status_code]
  cmp   rdi, BAD_REQUEST
  jl    .green

  cmp   rdi, INTERNAL_SERVER_ERROR
  jl    .yellow

  jmp   .red

  ; load green
.green:
  lea   rdi, [green_fg]
  jmp   .print

.yellow:
  lea   rdi, [yellow_fg]
  jmp   .print

.red:
  lea   rdi, [red_fg]

.print:
  ; print ansi to change color
  mov   rsi, 0
  call  print

  ; print status code
  mov   rdi, qword [response_status_code]
  call  to_string
  lea   rdi, [rax]
  mov   rsi, 0
  call  print

  ; back to default color
  lea   rdi, [default_fg]
  mov   rsi, 0
  call  print

  ret

; log current date and time
; @param  rdi: (optional) timestamp, if set to 0 get now
log_time:
  sub   rsp, 0x8 ; store timestamp

  cmp   rdi, 0
  je   .get_unix

  mov   qword [rsp], rdi

.get_unix:
  call  now
  cmp   rax, 0
  jl    .error

  mov   qword [rsp], rax

.print_time:
  mov   rdi, qword [rsp] 
  call  unix_to_date_time

  mov   rdi, qword [dt_year]
  call  to_string

  lea   rdi, [rax]
  mov   rsi, 0
  call  print

  mov   rdi, SLASH
  call  print_char

  cmp   qword [dt_month], 10
  jge   .no_zero_month

  mov   rdi, ZERO_CHAR
  call  print_char

.no_zero_month:
  mov   rdi, qword [dt_month]
  call  to_string

  lea   rdi, [rax]
  mov   rsi, 0
  call  print

  mov   rdi, SLASH
  call  print_char

  cmp   qword [dt_day], 10
  jge   .no_zero_day

  mov   rdi, ZERO_CHAR
  call  print_char

.no_zero_day:
  mov   rdi, qword [dt_day]
  call  to_string

  lea   rdi, [rax]
  mov   rsi, 0
  call  print

  mov   rdi, SPACE
  call	print_char

  mov   rdi, HYPHEN
  call	print_char

  mov   rdi, SPACE
  call	print_char

  cmp   qword [dt_hour], 10
  jge   .no_zero_hour

  mov   rdi, ZERO_CHAR
  call	print_char

.no_zero_hour:
  mov   rdi, qword [dt_hour]
  call  to_string

  lea   rdi, [rax]
  mov   rsi, 0
  call  print

  mov   rdi, COLON
  call  print_char

  cmp   qword [dt_minute], 10
  jge   .no_zero_minute

  mov   rdi, ZERO_CHAR
  call  print_char

.no_zero_minute:
  mov   rdi, qword [dt_minute]
  call  to_string

  lea   rdi, [rax]
  mov   rsi, 0
  call  print

  mov   rdi, COLON
  call  print_char

  cmp   qword [dt_second], 10
  jge   .no_zero_second

  mov   rdi, ZERO_CHAR
  call  print_char

.no_zero_second:
  mov   rdi, qword [dt_second]
  call  to_string

  lea   rdi, [rax]
  mov   rsi, 0
  call  print

  mov   rdi, SPACE
  call  print_char

  lea   rdi, [utc_str]
  mov   rsi, 0
  call  print

  mov   rax, SUCCESS_CODE
  jmp   .return
  
.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret


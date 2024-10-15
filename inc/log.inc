section .data
log_server_init_msg db SPACE, "[DEBUG] Listening and serving HTTP on :", NULL_CHAR
log_routes_available_msg db LINE_FEED, SPACE, "[DEBUG] Routes available:", NULL_CHAR

log_debug db SPACE, "[DEBUG]", SPACE, NULL_CHAR
log_arrow db SPACE, "->", SPACE, NULL_CHAR

log_pre       db SPACE, "[LOG]", SPACE, NULL_CHAR
log_delimiter db SPACE, VERTICAL_BAR, SPACE, NULL_CHAR

log_time      times LOG_TIME_SIZE db SPACE
log_duration  times LONG_SIZE+2   db SPACE

section .text
; initial log of the available routes and port the server is listening to
log_server_init:
  sub   rsp, 0x10 ; routes count for route logs

  lea   rdi, [log_server_init_msg]
  mov   rsi, 0
  call	print 

  mov   rdi, qword [port]
  call  to_string

  lea   rdi, [rax]
  mov   rsi, 0
  call	println

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
  lea   rax, [rsp+0x8]
  mov   rdi, [rax]
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

  mov   rax, 0
  add   rsp, 0x10
  ret

; store date string inside log_date
log_get_time:
  push  rbp
  mov   rbp, rsp

  mov   rdi, qword [start_tv_sec]
  call  unix_to_date_time

  mov   rdi, qword [dt_year]
  call  to_string

  lea   rsi, [rax]
  lea   rdi, [log_time]
  call  strcpy

  mov   al, SLASH
  stosb

  cmp   qword [dt_month], 10
  jge   .no_zero_month

  mov   al, ZERO_CHAR
  stosb

.no_zero_month:
  push  rdi

  mov   rdi, qword [dt_month]
  call  to_string

  lea   rsi, [rax]
  pop   rdi
  call  strcpy

  mov   al, SLASH
  stosb

  cmp   qword [dt_day], 10
  jge   .no_zero_day

  mov   al, ZERO_CHAR
  stosb

.no_zero_day:
  push  rdi

  mov   rdi, qword [dt_day]
  call  to_string

  lea   rsi, [rax]
  pop   rdi
  call  strcpy

  mov   al, SPACE
  stosb

  mov   al, HYPHEN
  stosb

  mov   al, SPACE
  stosb

  cmp   qword [dt_hour], 10
  jge   .no_zero_hour

  mov   al, ZERO_CHAR
  stosb

.no_zero_hour:
  push  rdi

  mov   rdi, qword [dt_hour]
  call  to_string

  lea   rsi, [rax]
  pop   rdi
  call  strcpy

  mov   al, COLON
  stosb

  cmp   qword [dt_minute], 10
  jge   .no_zero_minute

  mov   al, ZERO_CHAR
  stosb

.no_zero_minute:
  push  rdi

  mov   rdi, qword [dt_minute]
  call  to_string

  lea   rsi, [rax]
  pop   rdi
  call  strcpy

  mov   al, COLON
  stosb

  cmp   qword [dt_second], 10
  jge   .no_zero_second

  mov   al, ZERO_CHAR
  stosb

.no_zero_second:
  push  rdi

  mov   rdi, qword [dt_second]
  call  to_string

  lea   rsi, [rax]
  pop   rdi
  call  strcpy

  mov   al, SPACE
  stosb

  lea   rsi, [utc_str]
  call  strcpy

  leave
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

  mov   rcx, LOG_TIME_SIZE
  mov   rax, SPACE
  lea   rdi, [log_time]
  rep   stosb

  ; [LOG] 
  lea   rdi, [log_pre]
  mov   rsi, 0
  call  print

  ; log time
  call  log_get_time

  lea   rdi, [log_time]
  mov   rsi, LOG_TIME_SIZE
  call  print

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
  mov   rdi, qword [response_status_code]
  call  to_string
  lea   rdi, [rax]
  mov   rsi, 0
  call  print

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
  lea   rdi, [ERR_MALFORMED_REQUEST]
  mov   rsi, 0
  call  println

  mov   rax, 0
  ret


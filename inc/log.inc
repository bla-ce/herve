%ifndef LOG_INC
%define LOG_INC

; initial log of the available routes and port the server is listening to
log_server_init:
  sub   rsp, 0x8 ; routes count for route logs

  lea   rdi, [log_server_init_msg]
  mov   rsi, log_server_init_len
  call	print 

  mov   rdi, qword [port]
  call  to_string

  lea   rdi, [rax]
  call  strlen

  lea   rsi, [rdi]
  lea   rdi, [log_port]
  mov   rcx, rax
  rep   movsb

  ; add new line at the end
  mov   byte [rdi], LINE_FEED
  sub   rdi, rax
  mov   rsi, rax  ; length of port
  inc   rsi       ; include new line char
  call	print

  lea   rdi, [log_routes_available_msg]
  mov   rsi, log_routes_available_len
  call	print

  ; log routes - [DEBUG] METHOD ROUTE

  ; keep track of routes
  mov   qword [rsp], 0

  ; log new line
  lea   rdi, [log_new_line]
  mov   rsi, 1
  call	print

log_route_loop:
  ; log debug 
  lea   rdi, [log_debug]
  mov   rsi, log_debug_len
  call	print

  ; load routes_list
  lea   rsi, [routes_list]

  ; get route pos
  xor   rdx, rdx
  mov   rax, qword [rsp]
  mov   rbx, STRUCT_ROUTE_LEN
  mul   rbx

  ; go to route
  add   rsi, rax

  ; populate log url
  lea   rdi, [log_url]
  mov   rcx, URL_MAX_LEN
  rep   movsb

  ; populate method url
  lea   rdi, [log_method]
  mov   rcx, METHOD_MAX_LEN
  rep   movsb

  ; clear and add space after method and url
  lea   rdi, [log_url]
  call  strlen

  add   rdi, rax
  mov   rcx, URL_MAX_LEN
  sub   rcx, rax
  mov   al, SPACE
  rep   stosb

  lea   rdi, [log_method]
  call  strlen

  add   rdi, rax
  mov   rcx, METHOD_MAX_LEN
  sub   rcx, rax
  mov   al, SPACE
  rep   stosb

  ; log method
  lea   rdi, [log_method]
  mov   rsi, METHOD_MAX_LEN
  call	print

  ; log url
  lea   rdi, [log_url]
  mov   rsi, URL_MAX_LEN
  call	print
  
  ; log new line
  lea   rdi, [log_new_line]
  mov   rsi, 1
  call	print

  inc   qword [rsp]
  
  mov   rax, qword [rsp]
  cmp   rax, qword [routes_count]
  je    log_route_return
  
  jmp   log_route_loop

log_route_return:
  ; log new line
  lea   rdi, [log_new_line]
  mov   rsi, 1
  call	print

  mov   rax, 0
  add   rsp, 0x8
  ret

; store status code inside log_status
log_add_status:
  mov   eax, dword [response_status_code]
  cdqe

  mov   rdi, rax
  call  to_string
  
  lea   rsi, [rax]
  lea   rdi, [log_status]
  call  strcpy

  ret

; store duration inside log_duration
log_add_duration:
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
  jl    log_add_duration_loop

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
  
log_add_duration_loop:
  xor   rdx, rdx
  mov   rcx, 0xA              ; divisor, dividend in rax
  div   rcx                   ; quotient in rax and remainder in edx

  add   rdx, ZERO_CHAR

  mov   byte [rsi], dl

  dec   rsi

  cmp   rax, 0
  jne   log_add_duration_loop

  add   rsp, 0x8
  mov   rax, SUCCESS_CODE
  ret


log_request:
  ; clear log buffers
  mov   rcx, METHOD_MAX_LEN
  mov   rax, SPACE
  lea   rdi, [log_method]
  rep   stosb

  mov   rcx, URL_MAX_LEN
  mov   rax, SPACE
  lea   rdi, [log_url]
  rep   stosb

  mov   rcx, LONG_SIZE
  add   rcx, 2
  mov   rax, SPACE
  lea   rdi, [log_duration]
  rep   stosb

  lea   rdi, [request_method]
  call  strlen
  mov   rcx, rax

  ; make sure rcx is below METHOD_MAX_LEN
  cmp   rcx, METHOD_MAX_LEN
  mov   rax, METHOD_MAX_LEN
  cmovg rcx, rax

  lea   rsi, [request_method]
  lea   rdi, [log_method]
  rep   movsb

  lea   rdi, [request_url]
  call  strlen
  mov   rcx, rax

  ; make sure rcx is below URL_MAX_LEN
  cmp   rcx, URL_MAX_LEN
  mov   rax, URL_MAX_LEN
  cmovg rcx, rax

  lea   rsi, [request_url]

  mov   al, byte [rsi]
  cmp   al, NULL_CHAR
  je    jump_log_route

  lea   rdi, [log_url]
  rep   movsb

jump_log_route:

  call log_add_duration

  call log_add_status

  lea   rdi, [log_start]
  mov   rsi, log_len
  call	print 

  mov   rax, 0
  ret

log_malformed_request:
  lea   rdi, [ERR_MALFORMED_REQUEST]
  mov   rsi, 0
  call  println

  mov   rax, 0
  ret

%endif


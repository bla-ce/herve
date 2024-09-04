%ifndef LOG_INC
%define LOG_INC

log_server_init:
  sub   rsp, 0x8 ; routes count for route logs

  mov   rax, SYS_WRITE
  mov   rdi, STDOUT
  lea   rsi, [log_server_init_msg]
  mov   rdx, log_server_init_len
  syscall

  mov   rdi, qword [port]
  lea   rsi, [log_port]
  mov   rdx, LOG_PORT_LEN

  call  to_string

  ; add new line at the end
  add   rdi, LOG_PORT_LEN
  dec   rdi
  mov   byte [rdi], LINE_FEED
  sub   rdi, LOG_PORT_LEN
  inc   rdi

  mov   rsi, rdi
  mov   rax, SYS_WRITE
  mov   rdi, STDOUT

  mov   rdx, LOG_PORT_LEN
  syscall

  lea   rdi, [log_routes_available_msg]
  mov   rsi, log_routes_available_len
  call  debug

  ; log routes - [DEBUG] METHOD ROUTE

  ; keep track of routes
  mov   qword [rsp], 0

  ; log new line
  lea   rdi, [log_new_line]
  mov   rsi, 1
  call  debug

log_route_loop:
  ; log debug 
  lea   rdi, [log_debug]
  mov   rsi, log_debug_len
  call  debug

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
  call  get_length

  add   rdi, rax
  mov   rcx, URL_MAX_LEN
  sub   rcx, rax
  mov   al, SPACE
  rep   stosb

  lea   rdi, [log_method]
  call  get_length

  add   rdi, rax
  mov   rcx, METHOD_MAX_LEN
  sub   rcx, rax
  mov   al, SPACE
  rep   stosb

  ; log method
  lea   rdi, [log_method]
  mov   rsi, METHOD_MAX_LEN
  call  debug

  ; log url
  lea   rdi, [log_url]
  mov   rsi, URL_MAX_LEN
  call  debug
  
  ; log new line
  lea   rdi, [log_new_line]
  mov   rsi, 1
  call  debug

  inc   qword [rsp]
  
  mov   rax, qword [rsp]
  cmp   rax, qword [routes_count]
  je    log_route_return
  
  jmp   log_route_loop

log_route_return:
  mov   rax, 0
  add   rsp, 0x8
  ret

log_add_status:
  lea   rsi, [log_status]
  add   rsi, STATUS_CODE_LEN
  dec   rsi

  mov   eax, dword [response_status_code]
  
log_add_status_loop:
  xor   edx, edx
  mov   ecx, 0xA              ; divisor, dividend in rax
  div   ecx                   ; quotient in rax and remainder in edx

  add   edx, ZERO_CHAR

  mov   byte [rsi], dl

  dec   esi

  cmp   eax, 0
  jne   log_add_status_loop

  mov   rax, 0
  ret

log_add_duration:
  sub   rsp, 8 ; duration_str length
  mov   qword [rsp], 0

  lea   rsi, [log_duration]
  add   rsi, LONG_SIZE

  ; print ns backwards
  mov   byte [rsi], LOW_S_CHAR
  dec   rsi
  mov   byte [rsi], LOW_N_CHAR
  dec   rsi

  mov   rax, qword [duration]
  
log_add_duration_loop:
  xor   rdx, rdx
  mov   rcx, 0xA              ; divisor, dividend in rax
  div   rcx                   ; quotient in rax and remainder in edx

  add   rdx, ZERO_CHAR

  mov   byte [rsi], dl

  dec   rsi

  cmp   rax, 0
  jne   log_add_duration_loop

  add   rsp, 8
  mov   rax, 0
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
  call  get_length
  mov   rcx, rax

  lea   rsi, [request_method]
  lea   rdi, [log_method]
  rep   movsb

  lea   rdi, [request_url]
  call  get_length
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

  mov   rax, SYS_WRITE
  mov   rdi, STDOUT
  lea   rsi, [log_start]
  mov   rdx, log_len
  syscall

  mov   rax, 0
  ret

%endif


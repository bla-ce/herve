%ifndef LOG_INC
%define LOG_INC

%macro log_add_status 0
  lea   rsi, [log_status]
  add   rsi, STATUS_CODE_LEN
  dec   rsi

  mov   eax, dword [http_status_code]
  
%%loop:
  xor   edx, edx
  mov   ecx, 0xA              ; divisor, dividend in rax
  div   ecx                   ; quotient in rax and remainder in edx

  add   edx, ZERO_CHAR

  mov   byte [rsi], dl

  dec   esi

  cmp   eax, 0
  jne   %%loop
%endmacro

%macro log_add_duration 0
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
  
%%loop:
  xor   rdx, rdx
  mov   rcx, 0xA              ; divisor, dividend in rax
  div   rcx                   ; quotient in rax and remainder in edx

  add   rdx, ZERO_CHAR

  mov   byte [rsi], dl

  dec   rsi

  cmp   rax, 0
  jne   %%loop

  add   rsp, 8
%endmacro

%macro log_request 0
  ; clear log buffers
  mov   rcx, METHOD_MAX_LEN
  mov   rax, SPACE
  lea   rdi, [log_method]
  rep   stosb

  mov   rcx, ROUTE_MAX_LEN
  mov   rax, SPACE
  lea   rdi, [log_route]
  rep   stosb

  mov   rcx, LONG_SIZE
  add   rcx, 2
  mov   rax, SPACE
  lea   rdi, [log_duration]
  rep   stosb

  lea   rsi, [method]
  lea   rdi, [log_method]
  mov   rcx, [method_len] 
  rep   movsb

  lea   rsi, [route]

  mov   al, byte [rsi]
  cmp   al, NULL_CHAR
  je    jump_log_route

  lea   rdi, [log_route]
  mov   rcx, [route_len]
  rep   movsb

jump_log_route:

  log_add_duration

  log_add_status

  mov   rax, SYS_WRITE
  mov   rdi, STDOUT
  lea   rsi, [log_start]
  mov   rdx, log_len
  syscall
%endmacro

%endif


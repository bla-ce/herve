%ifndef SERVER_INC
%define SERVER_INC

%macro htons 1
  ; %1 -> port
  mov   eax, %1
  xchg  al, ah ; bswap 16-bit registers
%endmacro

%macro disallow_method 1
  ; %1 -> method
  mov   rsi, %1

  sub   rsp, 0x8

  push  rsi
  get_length rsi
  pop   rsi

  mov   qword [rsp], rax

  lea   rdi, [request_method]
  mov   rcx, rax
  rep   movsb

  ; check method arg, variable should be between GET and CONNECT
  mov   rdi, %1
  lea   rax, [GET]
  lea   rbx, [CONNECT]

  lea   rsi, [method_doesnt_exist]
  mov   rdx, method_doesnt_exist_len

  cmp   rdi, rax
  jl    error

  cmp   rdi, rbx
  jg    error

  dec   qword [rsp] ; remove null_char 
  mov   r10, qword [rsp]
  strpos methods_list, methods_list_len, request_method, r10

  inc   qword [rsp]

  lea   rsi, [method_doesnt_exist]
  mov   rdi, method_doesnt_exist_len
  cmp   rax, 0
  jl    error

  lea   rdi, [not_allowed_methods]
  add   rdi, rax

  lea   rsi, [request_method]
  mov   rcx, qword [rsp]
  rep   movsb

  xor   rax, rax
  add   rsp, 0x8
%endmacro

%macro extract_method 2
  ; %1 -> request pointer
  ; %2 -> request length
  sub   rsp, 8 ; current index
  mov   qword [rsp], 0

  mov   rsi, %1
  mov   rcx, %2
  xor   rax, rax

%%loop:
  lodsb
  cmp   al, SPACE
  je    %%return

  inc   qword [rsp]

  cmp   qword [rsp], rcx
  jl    %%loop

  ; bad request
  mov   rax, -1
  jmp   %%skip

%%return:
  dec   rsi
  mov   rax, qword [rsp]

  sub   rsi, rax
  lea   rdi, [request_method]
  mov   rcx, rax
  rep   movsb

%%skip:
  add   rsp, 8 
%endmacro

%macro check_method 2
  ; %1 -> method
  ; %2 -> method length
  sub   rsp, 8 

  lea   rsi, [error_method_too_big]
  mov   rdx, error_method_too_big_len

  mov   rcx, %2
  cmp   rcx, METHOD_MAX_LEN
  jg    error

  mov   qword [rsp], 0 ; string index

  strpos methods_list, methods_list_len, %1, %2
  cmp   rax, 0
  jge   method_exists
  
method_not_allowed:
  mov   rax, -1
  jmp   %%return

method_exists:
  mov   qword [rsp], rax

  lea   rdi, [methods_list]
  add   rdi, qword [rsp]
  add   rdi, %2

  ; make sure we checked the entire method
  mov   al, byte [rdi]
  cmp   al, SPACE
  jne   method_not_allowed

  mov   rax, %2
  sub   rdi, rax 
  dec   rdi
  mov   al, byte [rdi]
  cmp   al, SPACE
  jne   method_not_allowed

  ; check if the method is allowed
  strpos not_allowed_methods, methods_list_len, %1, %2
  cmp   rax, 0
  jge   method_not_allowed

  mov   rax, qword [rsp]

%%return:
  add   rsp, 8
%endmacro

%macro check_route 2
  ; %1 -> route
  ; %2 -> route length
  sub   rsp, 8 

  lea   rsi, [error_route_too_big]
  mov   rdx, error_route_too_big_len

  mov   rcx, %2
  cmp   rcx, URL_MAX_LEN
  jg    error

  mov   qword [rsp], rcx

  ; check if route ends by slash, if yes trim it
  mov   rsi, %1
  cmp   qword [rsp], 1
  je    %%jump

  add   rsi, qword [rsp]
  dec   rsi
  lodsb

  dec   rsi

  cmp   al, SLASH
  jne   %%dont_trim_slash
  
  mov   byte [rsi], NULL_CHAR
  dec   qword [rsp]
  dec   rsi
  
%%dont_trim_slash:
  sub   rsi, qword [rsp]
  inc   rsi

%%jump:
  strpos routes_list, routes_list_len, %1, %2
  cmp   rax, 0
  jge   route_found
  
route_not_found:
  mov   rax, -1
  jmp   %%return

route_found:
  mov   qword [rsp], rax

  lea   rdi, [routes_list]
  add   rdi, qword [rsp]
  add   rdi, %2

  ; make sure we checked the entire method
  mov   al, byte [rdi]
  cmp   al, NULL_CHAR
  jne   route_not_found

  mov   rax, %2
  sub   rdi, rax 
  dec   rdi
  mov   al, byte [rdi]
  cmp   al, NULL_CHAR
  jne   route_not_found

  inc   rdi

  get_length request_method

  ; check the method is correct
  ; rdi points to the beginning of the route
  add   rdi, URL_MAX_LEN
  lea   rsi, [request_method] 
  mov   rcx, rax
  rep   cmpsb

  jne   route_not_found

  add   rdi, METHOD_MAX_LEN
  sub   rdi, rax

  ; load callback
  lea   rdi, [callback_list]
  mov   rax, qword [rsp] 

  xor   rdx, rdx
  mov   rbx, STRUCT_ROUTE_LEN
  div   rbx

  xor   rdx, rdx
  mov   rbx, 8
  mul   rbx

  add   rdi, rax

  ; get callback
  mov   rax, [rdi]

  ; check it's not null
  cmp   rax, 0
  je    skip_exec

  call  rax

skip_exec:
  mov   rax, qword [rsp]
%%return:
  add   rsp, 8
%endmacro

%macro extract_route 2
  ; %1 -> request pointer
  ; %2 -> request length
  sub   rsp, 8 ; current index
  mov   qword [rsp], 0

  mov   rsi, %1
  mov   rcx, %2
  xor   rax, rax

%%loop:
  cmp   qword [rsp], URL_MAX_LEN
  jge   %%bad_request

  lodsb
  cmp   al, SPACE
  je    %%return

  cmp   al, QUESTION_MARK
  je    %%return

  cmp   al, NUMBER_SIGN
  je    %%return

  inc   qword [rsp]

  cmp   qword [rsp], rcx
  jl    %%loop

%%bad_request:
  ; bad request
  mov   rax, -1
  jmp   %%skip

%%return:
  dec   rsi
  mov   rax, qword [rsp]

  sub   rsi, rax
  lea   rdi, [request_url]
  mov   rcx, rax
  rep   movsb

%%skip:
  add   rsp, 8 
%endmacro


%macro add_route 3
  ; %1 -> method
  ; %2 -> route
  ; %3 -> callback
  sub   rsp, 16
  mov   qword [rsp], 0 ; method_len
  mov   qword [rsp+8], 0 ; route_len

  mov   qword [rel callback], %3

  lea   rsi, [error_route_too_big]
  mov   rdx, error_route_too_big_len

  mov   rcx, URL_MAX_LEN

  ; method_length
  get_length %1
  mov   qword [rsp], rax

  ; route_length
  get_length %2
  mov   qword [rsp+8], rax

  mov   rsi, %2
  cmp   qword [rsp+8], 1
  je    %%jump

  add   rsi, qword [rsp+8]
  dec   rsi
  lodsb

  dec   rsi

  cmp   al, SLASH
  jne   %%dont_trim_slash
  
  mov   byte [rsi], NULL_CHAR
  dec   qword [rsp+8]
  dec   rsi
  
%%dont_trim_slash:
  sub   rsi, qword [rsp+8]
  inc   rsi

%%jump:
  xor   rdx, rdx
  mov   rax, qword [routes_count]
  mov   rbx, STRUCT_ROUTE_LEN
  mul   rbx

  lea   rdi, [routes_list] 
  add   rdi, rax
  mov   rcx, qword [rsp+8]
  rep   movsb

  add   rdi, URL_MAX_LEN
  sub   rdi, qword [rsp+8]

  mov   rsi, %1
  mov   rcx, qword [rsp]
  rep   movsb

  ; store callback in the right index
  xor   rdx, rdx
  lea   rdi, [callback_list]

  mov   rax, qword [routes_count]
  mov   rbx, 8
  mul   rbx
  add   rdi, rax

  mov   rax, [rel callback]
  mov   qword [rdi], rax

  inc   qword [routes_count]

  add   rsp, 16
%endmacro

%macro server_init 1
  sub   rsp, 8
 
  mov   qword [rsp], 0  ; sockfd
  ; create socket
  mov   rax, SYS_SOCKET
  mov   edi, AF_INET
  mov   rsi, SOCK_STREAM
  mov   rdx, 0
  syscall

  lea   rsi, [error_creating_socket]
  mov   rdx, error_creating_socket_len

  cmp   rax, 0
  jl    error

  mov   qword [rsp], rax

  ; set socket options
  mov   rax, SYS_SETSOCKOPT
  mov   rdi, qword [rsp]
  mov   rsi, SOL_SOCKET
  mov   rdx, SO_REUSEPORT
  mov   r10, enable
  mov   r8, 4 
  syscall

  lea   rsi, [error_setting_socket_option]
  mov   rdx, error_setting_socket_option_len

  cmp   rax, 0
  jl    error

  mov   rax, SYS_SETSOCKOPT
  mov   rdi, qword [rsp]
  mov   rsi, SOL_SOCKET
  mov   rdx, SO_REUSEADDR
  mov   r10, enable
  mov   r8, 4 
  syscall

  lea   rsi, [error_setting_socket_option]
  mov   rdx, error_setting_socket_option_len

  cmp   rax, 0
  jl    error

  ; bind socket
  xor   rax, rax
  htons %1
  mov   dword [server_sin_port], eax
  mov   rax, SYS_BIND
  mov   rdi, qword [rsp]
  lea   rsi, [server_sin_family]
  mov   rdx, server_addrlen
  syscall

  lea   rsi, [error_binding]
  mov   rdx, error_binding_len

  cmp   rax, 0
  jl    error

  ; listen socket
  mov   rax, SYS_LISTEN
  mov   rdi, qword [rsp]
  mov   rsi, BACKLOG
  syscall

  lea   rsi, [error_listening]
  mov   rdx, error_listening_len

  cmp   rax, 0
  jl    error

  mov   rax, qword [rsp]

  add   rsp, 8
%endmacro

%macro run_server 1
  ; %1 -> sockfd
  sub   rsp, 16
  mov   rax, %1
  mov   qword [rsp], rax
  mov   qword [rsp+8], 0 ; client fd

%%loop:
  ; set end timer
  mov   rax, SYS_CLOCK_GETTIME
  mov   rdi, CLOCK_REALTIME
  lea   rsi, [end_tv_sec]
  syscall

  cmp   qword [start_tv_sec], 0
  je    timer_not_defined

  mov   rax, qword [end_tv_nsec]
  sub   rax, qword [start_tv_nsec]

  mov   qword [duration], rax

  log_request

timer_not_defined:
  ; clear route and method
  mov   rcx, URL_MAX_LEN
  xor   rax, rax
  lea   rdi, [request_url]
  rep   stosb

  mov   rcx, METHOD_MAX_LEN
  xor   rax, rax
  lea   rdi, [request_method]
  rep   stosb

  ; accept connection
  mov   rax, SYS_ACCEPT
  mov   rdi, qword [rsp]
  lea   rsi, [client_sin_family]
  lea   rdx, [client_addrlen]
  syscall

  lea   rsi, [error_accepting]
  mov   rdx, error_accepting_len

  cmp   rax, 0
  jl    error

  mov   qword [rsp+8], rax

  ; set start timer
  mov   rax, SYS_CLOCK_GETTIME
  mov   rdi, CLOCK_REALTIME
  lea   rsi, [start_tv_sec]
  syscall

  ; receive client request
  mov   rax, SYS_RECVFROM
  mov   rdi, qword [rsp+8]
  lea   rsi, [request]
  mov   rdx, REQUEST_MAX_LEN
  xor   r10, r10
  xor   r9, r9
  xor   r8, r8
  syscall

  lea   rsi, [error_reading_request]
  mov   rdx, error_reading_request_len

  cmp   rax, 0
  jl    error

  ; extract method
  extract_method request, rax
  cmp   rax, 0
  jge   move_to_check_method

  mov   rax, SYS_WRITE
  mov   rdi, qword [rsp+8]
  mov   rsi, response_400
  mov   rdx, response_400_len
  syscall

  mov   qword [response_status_code], BAD_REQUEST

  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp+8] 
  syscall

  jmp   %%loop

move_to_check_method:  
  get_length  request_method
  mov   r10, rax

  ; check if method is allowed
  check_method request_method, r10
  cmp   rax, 0
  jge   move_to_route

  mov   rax, SYS_WRITE
  mov   rdi, qword [rsp+8]
  mov   rsi, response_405
  mov   rdx, response_405_len
  syscall

  mov   qword [response_status_code], METHOD_NOT_ALLOWED

  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp+8] 
  syscall

  jmp   %%loop

move_to_route:
  get_length request
  mov   r10, rax
  get_length request_method
  mov   r9, rax

  lea   rsi, [request]
  add   rsi, r9 ; move to route (add one for the space)
  inc   rsi
  mov   rcx, r10
  sub   rcx, r9
  dec   rcx

  ; extract route
  extract_route rsi, rcx 
  cmp   rax, 0
  jge   test_route

  mov   rax, SYS_WRITE
  mov   rdi, qword [rsp+8]
  mov   rsi, response_400
  mov   rdx, response_400_len
  syscall

  mov   qword [response_status_code], BAD_REQUEST

  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp+8] 
  syscall

  jmp   %%loop

test_route:
  get_length request_url
  mov   r10, rax
  check_route request_url, r10
  cmp   rax, 0
  jge   move_to_rest_of_request

  mov   rax, SYS_WRITE
  mov   rdi, qword [rsp+8]
  mov   rsi, response_404
  mov   rdx, response_404_len
  syscall

  mov   qword [response_status_code], BAD_REQUEST

  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp+8] 
  syscall

  jmp   %%loop

move_to_rest_of_request:
  mov   qword [response_status_code], OK

  mov   rax, SYS_WRITE
  mov   rdi, qword [rsp+8]
  mov   rsi, response_200
  mov   rdx, response_200_len
  syscall

  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp+8]
  syscall

  jmp   %%loop

%endmacro

shutdown:
  ; close sockets
  mov   rax, SYS_CLOSE
  syscall

  ret

error:
  ; write error message
  mov   rax, SYS_WRITE
  mov   rdi, STDOUT
  syscall
 
  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp+8] 
  syscall

  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp]
  syscall

  mov   rax, SYS_EXIT
  mov   rdi, FAILURE_CODE
  syscall

%endif


%ifndef SERVER_INC
%define SERVER_INC

%macro htons 1
  ; %1 -> port
  mov   eax, %1
  xchg  al, ah ; bswap 16-bit registers
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
  mov   qword [method_len], rax

  sub   rsi, rax
  lea   rdi, [method]
  mov   rcx, rax
  rep   movsb

%%skip:
  add   rsp, 8 
%endmacro

%macro check_method 2
  ; %1 -> method
  ; %2 -> method length
  sub   rsp, 16 

  lea   rsi, [error_method_too_big]
  mov   rdx, error_method_too_big_len

  mov   rcx, %2
  cmp   rcx, METHOD_MAX_LEN
  jg    error

  mov   qword [rsp], 0 ; string index
  mov   qword [rsp+8], 0 ; substring index

  mov   rsi, %1
  xor   rax, rax
  lea   rdi, [methods_list]
  xor   rbx, rbx

%%loop:
  cmp   qword [rsp], methods_list_len
  je    method_not_allowed
  
  cld
  cmpsb
  jne   %%mismatch

  inc   qword [rsp]
  inc   qword [rsp+8]

  cmp   rcx, qword [rsp+8]
  je    method_exists
  jmp   %%loop

%%mismatch:
  dec   rsi
  sub   rsi, qword [rsp+8]
  mov   qword [rsp+8], 0

  inc   qword [rsp]

  jmp   %%loop
  
method_not_allowed:
  mov   rax, -1
  jmp   %%return

method_exists:
  ; make sure we checked the entire method
  mov   al, byte [rdi]
  cmp   al, SPACE
  jne   method_not_allowed

  ; check if the method is allowed
  mov   rsi, methods_list
  sub   rdi, rsi
  mov   rbx, rdi
  sub   rbx, %2

  lea   rsi, %1
  lea   rdi, [not_allowed_methods]
  add   rdi, rbx

  mov   rcx, %2
  rep   cmpsb
  je    method_not_allowed

  mov   rax, qword [rsp]

%%return:
  add   rsp, 16
%endmacro

%macro check_route 2
  ; %1 -> route
  ; %2 -> route length
  sub   rsp, 72 

  lea   rsi, [error_route_too_big]
  mov   rdx, error_route_too_big_len

  mov   rcx, %2
  cmp   rcx, ROUTE_MAX_LEN
  jg    error

  mov   qword [rsp], 0 ; string index
  mov   qword [rsp+8], 0 ; substring index

  ; check if last char is a slash
  mov   rdi, %1
  add   rdi, rcx
  dec   rdi
  lea   rsi, [rdi]
  lodsb
  sub   rsi, rcx

  cmp   rcx, 1
  je    %%jump

  mov   rbx, rcx
  dec   rbx

  cmp   al, SLASH
  cmove rcx, rbx

%%jump:
  xor   rax, rax
  lea   rdi, [routes_list]
  xor   rbx, rbx

%%loop:
  mov   rax, [routes_list_len]
  cmp   qword [rsp], rax
  je    route_not_found
  
  cld
  cmpsb
  jne   %%mismatch

  inc   qword [rsp+8]

  cmp   rcx, qword [rsp+8]
  je    route_found
  jmp   %%loop

%%mismatch:
  dec   rsi
  dec   rdi

  sub   rsi, qword [rsp+8]
  sub   rdi, qword [rsp+8]
  add   rdi, STRUCT_ROUTE_LEN

  mov   qword [rsp+8], 0

  inc   qword [rsp]

  jmp   %%loop
  
route_not_found:
  mov   rax, -1
  jmp   %%return

route_found:
  mov   al, byte [rdi]
  cmp   al, NULL_CHAR
  jne   route_not_found

  ; test method
  sub   rdi, rcx
  add   rdi, ROUTE_MAX_LEN

  lea   rsi, [method]

  cld
  mov   rcx, qword [method_len]
  rep   cmpsb
  jne   route_not_found

  mov   rax, qword [rsp]

%%return:
  add   rsp, 72
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
  cmp   qword [rsp], ROUTE_MAX_LEN
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
  mov   qword [route_len], rax

  sub   rsi, rax
  lea   rdi, [route]
  mov   rcx, rax
  rep   movsb

%%skip:
  add   rsp, 8 
%endmacro

%macro disallow_method 1
  ; %1 -> method
  mov   rsi, %1

  push rsi

  get_length rsi
  mov   rbx, rax

  pop rsi

  mov   [method_len], rbx

  lea   rdi, [method]
  mov   rcx, rbx
  rep   movsb

  push  rbx

  dec   qword [method_len] ; remove null_char 
  strpos methods_list, methods_list_len, method, [method_len]

  lea   rsi, [method_doesnt_exist]
  mov   rdi, method_doesnt_exist_len
  cmp   rax, 0
  jl    error

  pop   rbx

  lea   rdi, [not_allowed_methods]
  add   rdi, rax

  lea   rsi, [method]
  mov   rcx, rbx
  rep   movsb

  xor   rax, rax
%endmacro

%macro add_route 3
  ; %1 -> method
  ; %2 -> route
  ; %3 -> response
  sub   rsp, 24
  mov   qword [rsp], 0 ; route_len
  mov   qword [rsp+8], 0 ; method_len

  lea   rsi, [error_route_too_big]
  mov   rdx, error_route_too_big_len

  mov   rcx, ROUTE_MAX_LEN

  ; method_length
  get_length %1
  mov   qword [rsp], rax

  ; route_length
  get_length %2

  lea   rsi, [error_route_too_big]
  mov   rdx, error_route_too_big_len

  cmp   rax, ROUTE_MAX_LEN
  jg    error

  mov   qword [rsp+8], rax

  ; response_length
  get_length %3
  mov   qword [rsp+16], rax

  mov   rcx, qword [rsp+8]
  mov   rsi, %2

  lea   rdi, [routes_list]

  mov   rax, qword [routes_list_len]
  mov   rbx, STRUCT_ROUTE_LEN
  mul   rbx

  add   rdi, rax
  rep   movsb

  mov   rax, ROUTE_MAX_LEN
  sub   rax, qword [rsp+8]
  add   rdi, rax

  mov   rsi, %1
  mov   rcx, qword [rsp]
  rep   movsb

  cmp   qword [rsp+16], 0
  jle   %%continue 

  mov   rax, METHOD_MAX_LEN
  sub   rax, qword [rsp]
  add   rdi, rax

  mov   rsi, %3
  mov   rcx, qword [rsp+16]
  rep   movsb

%%continue:
  inc   qword [routes_list_len]

  lea   rsi, [error_max_route]
  mov   rdx, error_max_route_len

  mov   rax, qword [routes_list_len]
  cmp   rax, MAX_ROUTES_COUNT
  jg    error

  add   rsp, 24
%endmacro

%macro server_init 1
  sub   rsp, 64
  mov   qword [rsp], 0  ; sockfd
  ; create socket
  mov   rax, SYS_SOCKET
  mov   rdi, AF_INET
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

  add_route GET, root_route, ""

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
  add   rsp, 64
%endmacro

%macro run_server 1
  ; %1 -> sockfd
  sub   rsp, 128
  mov   rax, %1
  mov   qword [rsp], rax
  mov   qword [rsp+64], 0 ; client fd

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

  ; clear prev response
  mov   rcx, qword [response_len]
  mov   rax, NULL_CHAR
  lea   rdi, [response]
  rep   stosb

timer_not_defined:
  ; clear route and method
  mov   rcx, ROUTE_MAX_LEN
  xor   rax, rax
  lea   rdi, [route]
  rep   stosb

  mov   rcx, METHOD_MAX_LEN
  xor   rax, rax
  lea   rdi, [method]
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

  mov   qword [rsp+64], rax

  ; set start timer
  mov   rax, SYS_CLOCK_GETTIME
  mov   rdi, CLOCK_REALTIME
  lea   rsi, [start_tv_sec]
  syscall

  ; receive client request
  mov   rax, SYS_RECVFROM
  mov   rdi, qword [rsp+64]
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

  mov   qword [request_len], rax

  ; extract method
  extract_method request, [request_len] 
  cmp   rax, 0
  jge   move_to_check_method

  mov   rax, SYS_WRITE
  mov   rdi, qword [rsp+64]
  mov   rsi, response_400
  mov   rdx, response_400_len
  syscall

  mov   dword [http_status_code], BAD_REQUEST

  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp+64] 
  syscall

  jmp   %%loop

move_to_check_method:  
  ; check if method is allowed
  check_method method, [method_len]
  cmp   rax, 0
  jge   move_to_route

  mov   rax, SYS_WRITE
  mov   rdi, qword [rsp+64]
  mov   rsi, response_405
  mov   rdx, response_405_len
  syscall

  mov   dword [http_status_code], METHOD_NOT_ALLOWED

  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp+64] 
  syscall

  jmp   %%loop

move_to_route:
  lea   rsi, [request]
  add   rsi, [method_len] ; move to route (add one for the space)
  inc   rsi
  mov   rcx, [request_len]
  sub   rcx, [method_len]
  dec   rcx

  ; extract route
  extract_route rsi, rcx 
  cmp   rax, 0
  jge   test_route

  mov   rax, SYS_WRITE
  mov   rdi, qword [rsp+64]
  mov   rsi, response_400
  mov   rdx, response_400_len
  syscall

  mov   dword [http_status_code], BAD_REQUEST

  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp+64] 
  syscall

  jmp   %%loop

test_route:
  check_route route, [route_len]
  cmp   rax, 0
  jge   move_to_rest_of_request

  mov   rax, SYS_WRITE
  mov   rdi, qword [rsp+64]
  mov   rsi, response_404
  mov   rdx, response_404_len
  syscall

  mov   dword [http_status_code], 404

  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp+64] 
  syscall

  jmp   %%loop

move_to_rest_of_request:
  lea   rsi, [request]
  add   rsi, [route_len] ; move to rest of request (add one for the space)
  inc   rsi
  mov   rcx, [request_len]
  sub   rcx, [route_len]
  dec   rcx

  ; inside rax is the index of the route inside routes_list 
  mov   rbx, STRUCT_ROUTE_LEN
  mul   rbx

  add   rax, ROUTE_MAX_LEN
  add   rax, METHOD_MAX_LEN

  lea   rsi, [routes_list]
  add   rsi, rax

  lea   rdi, [response]
  mov   rcx, RESPONSE_MAX_LEN
  rep   movsb

  get_length response
  mov   qword [response_len], rax

  mov   dword [http_status_code], OK

  mov   rax, SYS_WRITE
  mov   rdi, qword [rsp+64]
  mov   rsi, response_200
  mov   rdx, response_200_len
  syscall

  lea   rsi, [response]
  lodsb

  cmp   al, SLASH
  je    serve_html

  mov   rax, SYS_WRITE
  mov   rdi, qword [rsp+64]
  lea   rsi, [response]
  mov   rdx, qword [response_len]
  syscall

  jmp   serve_string

serve_html:
  mov   rax, SYS_OPEN
  lea   rdi, [response]
  inc   rdi
  mov   rsi, O_RDONLY
  mov   rdx, 0
  syscall

  lea   rsi, [error_open]
  mov   rdx, error_open_len

  cmp   rax, 0
  jl    error

  mov   rdi, rax

  mov   rax, SYS_READ
  lea   rsi, [file]
  mov   rdx, FILE_MAX_LEN
  syscall

  lea   rsi, [error_read_file]
  mov   rdx, error_read_file_len

  cmp   rax, 0
  jl    error

  mov   qword [file_len], rax

  mov   rax, SYS_WRITE
  mov   rdi, qword [rsp+64]
  lea   rsi, [file]
  mov   rdx, qword [file_len]
  syscall

serve_string:
  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp+64] 
  syscall

  jmp   %%loop
%endmacro

%macro shutdown 0
  ; close sockets
  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp] 
  syscall

  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp+64] 
  syscall
%endmacro

error:
  ; write error message
  mov   rax, SYS_WRITE
  mov   rdi, STDOUT
  syscall
 
  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp+64] 
  syscall

  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp]
  syscall

  mov   rax, SYS_EXIT
  mov   rdi, FAILURE_CODE
  syscall

%endif


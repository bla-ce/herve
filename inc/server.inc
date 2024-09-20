%ifndef SERVER_INC
%define SERVER_INC

htons:
  ; rdi -> port
  ; return value: rax -> network port
  mov   rax, rdi
  xchg  al, ah ; bswap 16-bit registers
  ret

disallow_method:
  ; rdi -> method
  sub   rsp, 0x10

  mov   [rsp+0x8], rdi

  ; get length of the method
  call  get_length

  ; store the length of the method onto the stack
  mov   qword [rsp], rax

  ; check method arg, variable should be between GET and CONNECT
  lea   rax, [rsp+0x8]
  mov   rsi, [rax]
  lea   rax, [GET]
  lea   rbx, [CONNECT]

  lea   rdi, [ERR_METHOD_NOT_EXIST]
  cmp   rsi, rax
  jl    disallow_method_error

  cmp   rsi, rbx
  jg    disallow_method_error

  ; look if method is inside the list
  ; load request into rdx
  lea   rax, [rsp+0x8]
  mov   rdx, [rax]
  lea   rdi, [methods_list]
  mov   rsi, methods_list_len
  mov   rcx, qword [rsp]
  call  strpos

  ; add method to not allowed list
  lea   rdi, [not_allowed_methods]
  ; rdi points to the position of the method inside allowed list
  add   rdi, rax

  ; put method inside not allowed lists
  lea   rax, [rsp+0x8]
  mov   rsi, [rax]
  mov   rcx, qword [rsp]
  rep   movsb

  xor   rax, rax
  jmp   disallow_method_return

disallow_method_error:
  mov   [errno], rdi
  mov   rax, -1

disallow_method_return:
  ; clean stack
  add   rsp, 0x10
  ret

check_method:
  ; rdi -> method
  ; rsi -> method length
  sub   rsp, 0x18

  mov   qword [rsp], 0 ; string index
  mov   [rsp+0x8], rdi 
  mov   qword [rsp+0x10], rsi

  ; look for the method inside methods list
  lea   rdi, [methods_list]
  mov   rsi, methods_list_len
  lea   rax, [rsp+0x8]
  mov   rdx, [rax]
  mov   rcx, qword [rsp+0x10]
  call  strpos

  cmp   rax, 0
  jge   method_exists
  
method_not_allowed:
  mov   rax, -1
  jmp   check_method_return

method_exists:
  ; store method's position onto the stack
  mov   qword [rsp], rax

  ; go to the method
  lea   rdi, [methods_list]
  add   rdi, qword [rsp]

  ; check if we are the beginning of the method
  dec   rdi
  mov   al, byte [rdi]
  cmp   al, SPACE
  jne   method_not_allowed

  ; go to the end of the method
  inc   rdi
  add   rdi, qword [rsp+0x10]

  ; make sure the next byte is a space
  mov   al, byte [rdi]
  cmp   al, SPACE
  jne   method_not_allowed

  ; check if the method is allowed
  lea   rdi, [not_allowed_methods]
  mov   rsi, methods_list_len
  lea   rax, [rsp+0x8]
  mov   rdx, [rax]
  mov   rcx, qword [rsp+0x10] 
  call  strpos

  cmp   rax, 0
  jge   method_not_allowed

  ; return method's position
  mov   rax, qword [rsp]

check_method_return:
  add   rsp, 0x18
  ret

check_route:
  sub   rsp, 0x18 
  
  lea   rdi, [request_url]
  call  get_length

  mov   [rsp], rdi
  mov   qword [rsp+0x10], 0 ; route position inside list

  mov   rbx, rax

  ; remove query parameters, return url length
  mov   rsi, rbx
  call  trim_query_parameters
  cmp   rax, 0
  cmovg rbx, rax ; if rax > 0: found query param, exclude them

  mov   qword [rsp+0x8], rbx
  
  ; load url
  lea   rax, [rsp]
  mov   rsi, [rax]

  ; check if route is '/', if yes skip trim end slash
  cmp   qword [rsp+0x8], 1
  je    check_route_jump
  
  ; go to the end of the url
  add   rsi, qword [rsp+0x8] ; rsi points to the null_char

  ;  check if last char is slash
  dec   rsi
  cmp   byte [rsi], SLASH
  jne   check_route_jump
  
  ; if yes, change to null char and dec route length
  mov   byte [rsi], NULL_CHAR
  dec   qword [rsp+0x8]
  
check_route_jump:
  ; check if route is in routes_list
  xor   rbx, rbx
  lea   rdi, [routes_list]
  lea   rax, [rsp]
  mov   rsi, [rax]

next_route:
  mov   rcx, qword [rsp+0x8]
  cld
  rep   cmpsb
  je    route_found

not_entire:
  add   rdi, STRUCT_ROUTE_LEN
  sub   rdi, qword [rsp+0x8]
  add   rdi, rcx
  add   rsi, rcx
  sub   rsi, qword [rsp+0x8]

  inc   rbx
  cmp   rbx, qword [routes_count]
  jg    route_not_found

  jmp   next_route
  
route_not_found:
  mov   rax, -1
  jmp   check_route_return

route_found:
  ; make sure we checked the entire method
  mov   al, byte [rdi]
  cmp   al, NULL_CHAR
  jne   not_entire

  ; store position of the route
  xor   rdx, rdx
  mov   rax, STRUCT_ROUTE_LEN
  mul   rbx
  mov   qword [rsp+0x10], rax

  ; point to the end of the route inside the array
  lea   rdi, [routes_list]
  add   rdi, qword [rsp+0x8]
  add   rdi, qword [rsp+0x10]

  mov   rax, qword [rsp+0x8]
  sub   rdi, rax 
  dec   rdi
  mov   al, byte [rdi]
  cmp   al, NULL_CHAR
  jne   route_not_found

  inc   rdi

  push  rdi

  lea   rdi, [request_method]
  call  get_length

  pop   rdi

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
  mov   rax, qword [rsp+0x10]

  xor   rdx, rdx
  mov   rbx, STRUCT_ROUTE_LEN
  div   rbx

  xor   rdx, rdx
  mov   rbx, 8
  mul   rbx

  add   rdi, rax

  mov   rax, [rdi]

  cmp   rax, 0
  je    skip_exec

  mov   [rel callback], rax

skip_exec:
  mov   rax, qword [rsp+0x8]

check_route_return:
  add   rsp, 0x18
  ret

add_route:
  ; rdi -> method
  ; rsi -> route
  ; rdx -> callback
  sub   rsp, 0x20

  mov   qword [rsp], 0 ; method_len
  mov   qword [rsp+0x8], 0 ; route_len

  mov   [rsp+0x10], rdi 
  mov   [rsp+0x18], rsi

  ; store callback
  mov   qword [rel callback], rdx

  ; get method_length
  lea   rax, [rsp+0x10]
  mov   rdi, [rax]
  call  get_length
  mov   qword [rsp], rax

  ; get route_length
  lea   rax, [rsp+0x18]
  mov   rdi, [rax]
  call  get_length
  mov   qword [rsp+0x8], rax

  ; check if the length of the route is equal to 1
  lea   rax, [rsp+0x18]
  mov   rsi, [rax]
  cmp   qword [rsp+0x8], 1
  je    add_route_jump

  ; if not equal, extract last char to see if it's a slash
  add   rsi, qword [rsp+0x8]
  dec   rsi
  lodsb

  cmp   al, SLASH
  jne   add_route_dont_trim_slash
  
  ; if last char is a '/', remove it and add null char
  mov   byte [rsi], NULL_CHAR
  ; decrease route_length
  dec   qword [rsp+0x8]
  
add_route_dont_trim_slash:
  ; go back to beginning of the route
  sub   rsi, qword [rsp+0x8]

add_route_jump:
  ; go to next available space inside route_list
  xor   rdx, rdx
  mov   rax, qword [routes_count]
  mov   rbx, STRUCT_ROUTE_LEN
  mul   rbx

  lea   rdi, [routes_list] 
  add   rdi, rax
  ; move route inside the list
  mov   rcx, qword [rsp+0x8]
  rep   movsb

  ; move the method after the route
  add   rdi, URL_MAX_LEN
  sub   rdi, qword [rsp+0x8]

  ; load method inside rsi
  lea   rax, [rsp+0x10]
  mov   rsi, [rax]
  mov   rcx, qword [rsp]
  rep   movsb

  ; store callback in the right index
  xor   rdx, rdx
  lea   rdi, [callback_list]

  ; compute callback index
  mov   rax, qword [routes_count]
  mov   rbx, 8 ; size of qword
  mul   rbx
  add   rdi, rax

  mov   rax, [rel callback]
  mov   qword [rdi], rax

  ; clear callback
  mov   qword [callback], 0

  ; increase number of routes
  inc   qword [routes_count]

  add   rsp, 0x20
  ret

server_init:
  ; rdi -> port
  ; return value: rax -> socket fd | error
  sub   rsp, 0x8

  mov   qword [port], rdi
  mov   qword [rsp], 0  ; sockfd

  ; check if port is in bounds
  lea   rdi, [ERR_PORT_BOUNDS]
  cmp   qword [port], PORT_MIN
  jl    server_init_error

  cmp   qword [port], PORT_MAX
  jg    server_init_error

  ; create socket
  mov   rax, SYS_SOCKET
  mov   edi, AF_INET
  mov   rsi, SOCK_STREAM
  mov   rdx, 0
  syscall

  lea   rdi, [ERR_CREAT_SOCKET]
  cmp   rax, 0
  jl    server_init_error

  ; store server fd
  mov   qword [rsp], rax

  ; set socket options
  mov   rax, SYS_SETSOCKOPT
  mov   rdi, qword [rsp]
  mov   rsi, SOL_SOCKET
  mov   rdx, SO_REUSEPORT
  mov   r10, enable
  mov   r8, 4 
  syscall

  lea   rdi, [ERR_SET_SOCKOPT]
  cmp   rax, 0
  jl    server_init_error

  mov   rax, SYS_SETSOCKOPT
  mov   rdi, qword [rsp]
  mov   rsi, SOL_SOCKET
  mov   rdx, SO_REUSEADDR
  mov   r10, enable
  mov   r8, 4 
  syscall

  lea   rdi, [ERR_SET_SOCKOPT]
  cmp   rax, 0
  jl    server_init_error

  ; convert host port to network
  xor   rax, rax
  mov   rdi, qword [port]
  call  htons

  ; bind socket
  mov   dword [server_sin_port], eax
  mov   rax, SYS_BIND
  mov   rdi, qword [rsp]
  lea   rsi, [server_sin_family]
  mov   rdx, server_addrlen
  syscall

  lea   rdi, [ERR_BIND_SOCKET]
  cmp   rax, 0
  jl    server_init_error

  ; listen socket
  mov   rax, SYS_LISTEN
  mov   rdi, qword [rsp]
  xor   rsi, rsi
  syscall

  lea   rdi, [ERR_LISTEN_SOCKET]
  cmp   rax, 0
  jl    server_init_error

  mov   rax, qword [rsp]
  jmp   server_init_return

server_init_error:
  mov   [errno], rdi

  ; check if socket have been opened
  cmp   qword [rsp], 0
  jle   skip_close_socket

  ; if yes close it
  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp]
  syscall
  ; don't need to check for error, we exit anyway

skip_close_socket:
  mov   rax, -1

server_init_return:
  add   rsp, 0x8
  ret

run_server:
  ; rdi -> sockfd
  sub   rsp, 0x8

  mov   qword [rsp], rdi

  call  log_server_init

run_server_loop:
  ; set end timer
  mov   rax, SYS_CLOCK_GETTIME
  mov   rdi, CLOCK_REALTIME
  lea   rsi, [end_tv_sec]
  syscall

  ; check if we have set start timer
  cmp   qword [start_tv_sec], 0
  je    timer_not_defined

  ; if yes, compute the difference
  mov   rax, qword [end_tv_nsec]
  sub   rax, qword [start_tv_nsec]

  mov   qword [duration], rax

  ; clear callback
  mov   qword [callback], 0

  ; log previous request
  call  log_request

timer_not_defined:
  ; clear response, route and method
  mov   rcx, RESPONSE_MAX_LEN
  xor   rax, rax
  lea   rdi, [response]
  rep   stosb

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

  mov   qword [request_client], rax

  inc   qword [active_connections]
  mov   rax, qword [active_connections]
  cmp   rax, qword [max_connections]
  jg    server_unavailable

  ; set start timer
  mov   rax, SYS_CLOCK_GETTIME
  mov   rdi, CLOCK_REALTIME
  lea   rsi, [start_tv_sec]
  syscall

  ; receive client request
  mov   rax, SYS_RECVFROM
  mov   rdi, qword [request_client]
  lea   rsi, [request]
  mov   rdx, REQUEST_MAX_LEN
  xor   r10, r10
  xor   r9, r9
  xor   r8, r8
  syscall

  mov   qword [request_len], rax

  ; parse_request
  call  parse_request
  cmp   rax, 0
  jl    error_request

  ; verify request
  call  verify_request

  ; callback
  cmp   qword [callback], 0
  je    skip_callback

  lea   rdi, [request]
  call  [rel callback]

skip_callback:
  mov   rax, SYS_CLOSE
  mov   rdi, qword [request_client] 
  syscall
  
  dec   qword [active_connections]

  jmp   run_server_loop

error_request:
  mov   rax, SYS_CLOSE
  mov   rdi, qword [request_client] 
  syscall
  
  dec   qword [active_connections]

  ; jump to next request, skipping logs
  jmp   timer_not_defined

  ; shutdown server
  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp]
  syscall

  add   rsp, 0x8
  ret

server_unavailable:
  mov   qword [response_status_code], SERVICE_UNAVAILABLE
  mov   rdi, qword [request_client]
  lea   rsi, STR_SERVICE_UNAVAILABLE
  call  send_error_response

  jmp   error_request

trim_query_parameters:
  ; rdi -> request_url
  ; rsi -> route_length
  ; return value: rax -> url length
  sub   rsp, 0x8 ; url length

  mov   qword [rsp], rsi

  mov   rsi, QUESTION_MARK
  call  find_next_char
  cmp   rax, 0
  jle   trim_query_return

  ; if found query, add null char
  mov   qword [rsp], rax

  lea   rdi, [request_url]
  add   rdi, rax
  mov   al, NULL_CHAR
  stosb

trim_query_return:
  mov   rax, qword [rsp]

  add   rsp, 0x8
  ret

set_max_connections:
  ; rdi -> max connections
  mov   qword [max_connections], rdi

  ret

%endif


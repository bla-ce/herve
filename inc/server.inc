%ifndef SERVER_INC
%define SERVER_INC

htons:
  ; rdi -> port
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
  mov   rdi, [rax]
  lea   rax, [GET]
  lea   rbx, [CONNECT]

  ; load error messages
  lea   rsi, [method_doesnt_exist]
  mov   rdx, method_doesnt_exist_len

  cmp   rdi, rax
  jl    error

  cmp   rdi, rbx
  jg    error

  ; look if method is inside the list
  ; load request into rdx
  lea   rax, [rsp+0x8]
  mov   rdx, [rax]
  lea   rdi, [methods_list]
  mov   rsi, methods_list_len
  mov   rcx, qword [rsp]
  call  strpos

  ; load error messages if couldn't find the method
  lea   rsi, [method_doesnt_exist]
  mov   rdi, method_doesnt_exist_len
  cmp   rax, 0
  jl    error

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

  ; load error messages
  lea   rsi, [error_method_too_big]
  mov   rdx, error_method_too_big_len

  ; check the length of the method
  mov   rcx, qword [rsp+0x10]
  cmp   rcx, METHOD_MAX_LEN
  jg    error

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


; TODO: CLEAN
check_route:
  ; rdi -> route
  ; rsi -> route length
  sub   rsp, 0x18 

  mov   qword [rsp], rsi
  mov   [rsp+0x8], rdi
  mov   qword [rsp+0x10], 0 ; route position inside list

  ; remove query parameters
  lea   rdi, [request_url]
  call  trim_query_parameters

  cmp   rax, 0
  cmovg rsi, rax

  mov   qword [rsp], rsi
  
  ; check if route ends by slash, if yes trim it
  lea   rax, [rsp+0x8]
  mov   rsi, [rax]
  cmp   qword [rsp], 1
  je    check_route_jump

  add   rsi, qword [rsp]
  dec   rsi
  lodsb

  dec   rsi

  cmp   al, SLASH
  jne   dont_trim_slash
  
  mov   byte [rsi], NULL_CHAR
  dec   qword [rsp]
  dec   rsi
  
dont_trim_slash:
  sub   rsi, qword [rsp]
  inc   rsi

check_route_jump:
  lea   rdi, [routes_list]
  mov   rsi, routes_list_len
  lea   rax, [rsp+0x8]
  mov   rdx, [rax]
  mov   rcx, qword [rsp]
  call  strpos
  cmp   rax, 0
  jge   route_found
  
route_not_found:
  mov   rax, -1
  jmp   check_route_return

route_found:
  mov   qword [rsp+0x10], rax

  lea   rdi, [routes_list]
  add   rdi, qword [rsp+0x10]
  add   rdi, qword [rsp]

  ; make sure we checked the entire method
  mov   al, byte [rdi]
  cmp   al, NULL_CHAR
  jne   route_not_found

  mov   rax, qword [rsp]
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

  lea   rdi, [request]
  call  rax

skip_exec:
  mov   rax, qword [rsp]

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

  ; increase number of routes
  inc   qword [routes_count]

  add   rsp, 0x20
  ret

server_init:
  ; rdi -> port
  sub   rsp, 0x8

  mov   qword [port], rdi
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

  ; log previous request
  call  log_request

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

  mov   qword [request_client], rax

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

  lea   rsi, [error_reading_request]
  mov   rdx, error_reading_request_len

  cmp   rax, 0
  jl    error

  mov   qword [request_len], rax

  ; parse_request
  call  parse_request

  ; verify request
  call  verify_request

  mov   rdi, qword [request_client]
  call  send_response

  ; close connection
  ; TODO: might not want to do it
  mov   rax, SYS_CLOSE
  mov   rdi, qword [request_client] 
  syscall
 
  ; jump to next request
  jmp   run_server_loop

  ; shutdown server
  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp]
  syscall

  add   rsp, 0x8
  ret

; TODO: this thing probably don't work as expected
error:
  ; write error message
  mov   rdi, rsi
  mov   rsi, rdx
  call  debug 
 
  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp+0x8]
  syscall

  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp]
  syscall

  mov   rax, SYS_EXIT
  mov   rdi, FAILURE_CODE
  syscall

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
%endif


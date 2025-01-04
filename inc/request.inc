%ifndef REQUEST_INC
%define REQUEST_INC

; parse the string representation of the request onto the request struct
; @param  rdi: pointer to request
; @return rax: code
parse_request:
  sub   rsp, 0x8

  mov   [rsp], rdi

  ; extract method: look for the next space
  lea   rdi, [rsi]
  mov   rsi, SPACE
  call  find_next_char

  ; if < 0, bad request
  cmp   rax, 0x0
  jl    .error 

  ; compare the length of the method with METHOD_MAX_LEN
  cmp   rax, METHOD_MAX_LEN
  ; if bigger, send bad request
  jg    .error

  ; load method
  lea   rdi, [request_method]
  mov   rcx, rax
  mov   rsi, [rsp] 
  rep   movsb

  ; method url HTTP/n.n 
  ;       | inc rsi to go to first char of the url
  inc   rsi

  ; save rsi pointer to the stack
  mov   [rsp], rsi

  ; extract url: look for the next space
  lea   rdi, [rsi]
  mov   rsi, SPACE
  call  find_next_char

  cmp   rax, 0x0
  jl    .error

  ; verify url length
  cmp   rax, URL_MAX_LEN
  jg    .error

  ; load url
  lea   rdi, [request_url]
  mov   rcx, rax
  mov   rsi, [rsp]
  rep   movsb

  ; inc rsi to go to the protocol 
  inc   rsi

  ; extract proto
  ; save rsi pointer to the stack
  mov   [rsp], rsi

  lea   rdi, [rsi]
  mov   rsi, CARRIAGE_RETURN
  call  find_next_char  ; find next \r to get the entire protocol
  cmp   rax, 0x0
  jl    .error

  ; load protocol
  mov   rcx, rax
  lea   rdi, [request_proto]
  mov   rsi, [rsp]
  rep   movsb

  ; save rsi pointer to the stack
  mov   [rsp], rsi

  ; parse proto -> format: HTTP/0.0
  lea   rdi, [request_proto]
  mov   rsi, SLASH
  call  find_next_char ; go to '/' after 'HTTP'
  cmp   rax, 0x0
  jl    .error

  mov   rcx, rax

  ; rsp points to the end of the protocol
  mov   rsi, [rsp]

  ; go back to slash
  sub   rsi, rcx

  ; mov to proto major
  inc   rsi

  xor   rdi, rdi
  mov   dil, byte [rsi]
  call  ascii_to_int

  cmp   rax, 0
  jl    .error

  mov   dword [request_proto_major], eax

  ; rsi points to the proto major, mov to proto minor
  add   rsi, 0x2

  xor   rdi, rdi
  mov   dil, byte [rsi]
  call  ascii_to_int

  cmp   rax, 0
  jl    .error

  mov   dword [request_proto_minor], eax

  inc   rsi

  ; mov to new line (skip 2 char \r\n)
  add   rsi, 0x2

  ; save headers onto the stack
  mov   [rsp], rsi

  ; find the end of the headers (\r\n\r\n)
  lea   rdi, [rsi]
  lea   rsi, [post_headers]
  call  strpos

  cmp   rax, 0
  jl    .error

  ; verify headers overflow
  cmp   rax, HEADERS_MAX_LEN
  jg    .error
   
  mov   rcx, rax
  mov   rsi, [rsp]
  lea   rdi, [request_headers]
  rep   movsb

  ; get body
  ; move to new line (skip 4 char \r\n\r\n)
  add   rsi, 0x4

  mov   [rsp], rsi

  ; find the end of the request
  lea   rdi, [rsi]
  mov   rsi, NULL_CHAR
  call  find_next_char
  cmp   rax, 0x0
  jl    .error

  cmp   rax, BODY_MAX_LEN
  jg    .error

  lea   rdi, [request_body]
  mov   rcx, rax
  mov   rsi, [rsp]
  rep   movsb

  mov   rax, 0

  jmp   .return

.error:
  mov   qword [response_status_code], BAD_REQUEST
  mov   rdi, qword [request_client]
  lea   rsi, [STR_BAD_REQUEST]
  call  send_error_response

  mov   rax, FAILURE_CODE
  jmp   .return

.return:
  add   rsp, 0x8
  ret


; TODO: pass the request struct as an arg
; verifies the request, making sure the route exists and is allowed
verify_request:
  call  check_method
  cmp   rax, 0
  jl    .method_not_allowed

  call  check_route
  cmp   rax, 0
  jl    .not_found

  mov   qword [response_status_code], OK
  mov   rax, 0

  jmp   .return

.method_not_allowed:
  mov   qword [response_status_code], METHOD_NOT_ALLOWED

  mov   rdi, qword [request_client]
  lea   rsi, [STR_METHOD_NOT_ALLOWED]
  call  send_error_response

  mov   rax, -1

  jmp   .return

.not_found:
  mov   qword [response_status_code], NOT_FOUND

  mov   rdi, qword [request_client]
  lea   rsi, [STR_NOT_FOUND]
  call  send_error_response

  mov   rax, -1

  jmp   .return

.return:
  ret

get_body:
  ; rdi -> request
  lea   rax, [request_body]
  ret

check_method:
  sub   rsp, 0x18

  lea   rdi, [request_method]
  call  strlen

  mov   qword [rsp], 0 ; string index
  mov   [rsp+0x8], rdi 
  mov   qword [rsp+0x10], rax

  ; look for the method inside methods list
  lea   rdi, [methods_list]
  mov   rsi, [rsp+0x8]
  call  strpos

  cmp   rax, 0
  jge   .method_exists
  
.method_not_allowed:
  mov   rax, -1
  jmp   .return

.method_exists:
  ; store method's position onto the stack
  mov   qword [rsp], rax

  ; go to the method
  lea   rdi, [methods_list]
  add   rdi, qword [rsp]

  ; check if we are the beginning of the method
  dec   rdi
  mov   al, byte [rdi]
  cmp   al, SPACE
  jne   .method_not_allowed

  ; go to the end of the method
  inc   rdi
  add   rdi, qword [rsp+0x10]

  ; make sure the next byte is a space
  mov   al, byte [rdi]
  cmp   al, SPACE
  jne   .method_not_allowed

  ; check if the method is allowed
  lea   rdi, [not_allowed_methods]
  mov   rsi, [rsp+0x8]
  call  strpos

  cmp   rax, 0
  jge   .method_not_allowed

  ; return method's position
  mov   rax, qword [rsp]

.return:
  add   rsp, 0x18
  ret

check_route:
  sub   rsp, 0x18 
  
  lea   rdi, [request_url]
  call  strlen

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
  mov   rsi, [rsp]

  ; check if route is '/', if yes skip trim end slash
  cmp   qword [rsp+0x8], 1
  je    .skip_trim
  
  ; go to the end of the url
  add   rsi, qword [rsp+0x8] ; rsi points to the null_char

  ;  check if last char is slash
  dec   rsi
  cmp   byte [rsi], SLASH
  jne   .skip_trim
  
  ; if yes, change to null char and dec route length
  mov   byte [rsi], NULL_CHAR
  dec   qword [rsp+0x8]
  
.skip_trim:
  ; check if route is in routes_list
  xor   rbx, rbx
  lea   rdi, [routes_list]
  mov   rsi, [rsp]

.next_route:
  mov   rcx, qword [rsp+0x8]
  cld
  rep   cmpsb
  je    .route_found

.not_entire:
  add   rdi, STRUCT_ROUTE_LEN
  sub   rdi, qword [rsp+0x8]
  add   rdi, rcx
  add   rsi, rcx
  sub   rsi, qword [rsp+0x8]

  inc   rbx
  cmp   rbx, qword [routes_count]
  jg    .route_not_found

  jmp   .next_route
  
.route_not_found:
  mov   rax, -1
  jmp   .return

.route_found:
  ; make sure we checked the entire method
  mov   al, byte [rdi]
  cmp   al, NULL_CHAR
  jne   .not_entire

  ; store position of the route
  xor   rdx, rdx
  mov   rax, STRUCT_ROUTE_LEN
  mul   rbx
  mov   qword [rsp+0x10], rax

  lea   rdi, [request_method]
  call  strlen

  ; check the method is correct
  lea   rdi, [routes_list]
  add   rdi, qword [rsp+0x10]
  add   rdi, URL_MAX_LEN
  lea   rsi, [request_method] 
  mov   rcx, rax
  rep   cmpsb

  jne   .route_not_found

  ; load callback
  lea   rdi, [callback_list]
  mov   rax, qword [rsp+0x10]

  ; get index of the route
  xor   rdx, rdx
  mov   rbx, STRUCT_ROUTE_LEN
  div   rbx

  ; go to callback inside callback_list
  xor   rdx, rdx
  mov   rbx, 8
  mul   rbx

  add   rdi, rax

  mov   rax, [rdi]

  ; check route has a callback
  cmp   rax, 0
  je    .skip_exec

  mov   [rel callback], rax

.skip_exec:
  mov   rax, qword [rsp+0x8]

.return:
  add   rsp, 0x18
  ret

; extracts the value of a specified query parameter from a given URL string
; @param  rdi: query parameter
; @return rax: pointer to value
get_param:
  ; init stack
  sub   rsp, 0x10
  mov   [rsp], rdi

  ; load query params
  lea   rdi, [query_params]
  mov   rsi, [rsp]

  ; check if parameter is in url
  call  strpos
  mov   rbx, rax

  cmp   rbx, 0
  jl    .error

  ; go to param
  lea   rdi, [query_params]
  add   rdi, rbx

  ; save param position onto the stack
  mov   [rsp], rdi

  ; go to the next equal
  mov   rsi, EQUAL_SIGN
  call  find_next_char
  add   rdi, rax

  ; jump after equal
  inc   rdi

  ; save value position onto the stack
  mov   [rsp], rdi

  ; find the next '&' or '\0'
  mov   rsi, AMPERSAND
  call  find_next_char
  mov   rbx, rax

  mov   rsi, NULL_CHAR
  call  find_next_char

  cmp   rbx, 0
  cmovg rcx, rbx

  cmp   rcx, rax
  cmovg rcx, rax

  cmp   rcx, 0
  jl    .error

  ; save param into buffer
  lea   rsi, [rdi] 
  xor   rdi, rdi
  lea   rdi, [query]
  mov   rcx, rax
  rep   movsb

  ; clean stack
  add   rsp, 0x10

  ; return param
  lea   rax, [query]
  ret

.error:
  mov   rax, FAILURE_CODE
  add   rsp, 0x10
  ret

%endif


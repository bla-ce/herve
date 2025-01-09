section .data
  ; constants
  METHOD_MAX_LEN        equ 0x8
  PROTO_MAX_LEN         equ 0x9
  HOST_MAX_LEN          equ 0xFF
  QUERY_MAX_LEN         equ 0x20
  QUERY_MAX_VALUE_LEN   equ 0x10
  URL_MAX_LEN           equ 0x30
  BODY_MAX_LEN          equ 0x2000

  request_struct:
    req_client      dq 0
    req_len         dq 0
    req_method      dq 0
    req_url         dq 0
    req_proto       dq 0
    req_headers     dq 0
    req_body        dq 0
    req_content_len dq 0
    req_host        dq 0
    req_close       db 0
  request_struct_end:

  REQ_STRUCT_LEN equ request_struct_end - request_struct

  ; offsets
  REQ_OFF_CLIENT      equ 0x0
  REQ_OFF_LEN         equ REQ_OFF_CLIENT + 0x8
  REQ_OFF_METHOD      equ REQ_OFF_LEN + 0x8
  REQ_OFF_URL         equ REQ_OFF_METHOD + 0x8
  REQ_OFF_PROTO       equ REQ_OFF_URL + 0x8
  REQ_OFF_HEADERS     equ REQ_OFF_PROTO + 0x8
  REQ_OFF_BODY        equ REQ_OFF_HEADERS + 0x8
  REQ_OFF_CONTENT_LEN equ REQ_OFF_BODY + 0x8
  REQ_OFF_HOST        equ REQ_OFF_CONTENT_LEN + 0x8
  REQ_OFF_CLOSE       equ REQ_OFF_HOST + 0x8
  
section .text
; parse the string representation of the request onto the request struct
; @param  rdi: pointer to request str
; @param  rsi: client fd
; @param  rdx: request length
; @return rax: pointer to request struct
parse_request:
  sub   rsp, 0x20

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  cmp   rdx, 0
  jle   .error

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx

  ; malloc struct
  mov   rdi, REQ_STRUCT_LEN
  call  malloc

  cmp   rdi, 0
  jl    .error

  ; save pointer
  mov   [rsp+0x18], rax

  ; save request data
  mov   rdi, [rsp+0x8]
  mov   [rax+REQ_OFF_CLIENT], rdi

  mov   rdi, [rsp+0x10]
  mov   [rax+REQ_OFF_LEN], rdi

  ; extract method: look for the next space
  mov   rdi, [rsp]
  mov   rsi, SPACE
  call  find_next_char

  ; if < 0, bad request
  cmp   rax, 0x0
  jl    .error 

  ; compare the length of the method with METHOD_MAX_LEN
  cmp   rax, METHOD_MAX_LEN
  ; if bigger, send bad request
  jg    .error

  mov   qword [rsp+0x10], rax

  ; malloc method
  mov   rdi, rax
  call  malloc

  cmp   rax, 0
  jle   .error

  mov   rdi, [rsp+0x18]
  mov   [rdi+REQ_OFF_METHOD], rax

  ; load method
  mov   rdi, rax
  mov   rcx, qword [rsp+0x10]
  mov   rsi, [rsp] 
  rep   movsb

  ; method url HTTP/n.n 
  ;       | inc rsi to go to first char of the url
  inc   rsi

  ; save rsi pointer to the stack
  mov   [rsp], rsi

  ; extract url: look for the next space
  xor   rdi, rdi
  lea   rdi, [rsi]
  mov   rsi, SPACE
  call  find_next_char

  cmp   rax, 0x0
  jl    .error

  ; verify url length
  cmp   rax, URL_MAX_LEN
  jg    .error

  mov   qword [rsp+0x10], rax

  mov   rdi, rax
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x18]
  mov   [rdi+REQ_OFF_URL], rax

  ; load url
  mov   rdi, rax
  mov   rcx, qword [rsp+0x10]
  mov   rsi, [rsp]
  rep   movsb

  ; inc rsi to go to the protocol 
  inc   rsi

  ; extract proto
  ; save rsi pointer to the stack
  mov   [rsp], rsi

  xor   rdi, rdi
  lea   rdi, [rsi]
  mov   rsi, CARRIAGE_RETURN
  call  find_next_char  ; find next \r to get the entire protocol
  cmp   rax, 0x0
  jl    .error

  mov   qword [rsp+0x10],rax

  ; malloc request protocol
  mov   rdi, PROTO_MAX_LEN
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x18]
  mov   [rdi+REQ_OFF_PROTO], rax

  ; load protocol
  mov   rdi, rax
  mov   rcx, qword [rsp+0x10]
  mov   rsi, [rsp]
  rep   movsb

  ; save rsi pointer to the stack
  mov   [rsp], rsi

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

  mov   qword [rsp+0x10], rax
   
  mov   rdi, rax
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x18]
  mov   [rdi+REQ_OFF_HEADERS], rax

  mov   rdi, rax
  mov   rcx, qword [rsp+0x10]
  mov   rsi, [rsp]
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

  mov   qword [rsp+0x10], rax
  mov   rdi, [rsp+0x18]
  mov   [rdi+REQ_OFF_CONTENT_LEN], rax

  mov   rdi, rax
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x18]
  mov   [rdi+REQ_OFF_BODY], rax

  mov   rdi, rax
  mov   rcx, qword [rsp+0x10]
  mov   rsi, [rsp]
  rep   movsb

  mov   rax, [rsp+0x18]

  jmp   .return

.error:
  mov   qword [response_status_code], BAD_REQUEST
  mov   rdi, qword [request_client]
  lea   rsi, [STR_BAD_REQUEST]
  call  send_error_response

  mov   rax, FAILURE_CODE
  jmp   .return

.return:
  add   rsp, 0x20
  ret


; verifies the request, making sure the route exists and is allowed
; @param  rdi: pointer to request struct
; @return rax: return code
verify_request:
  sub   rsp, 0x8

  mov   [rsp], rdi

  add   rdi, REQ_OFF_METHOD
  call  check_method
  cmp   rax, 0
  jl    .method_not_allowed

  mov   rdi, [rsp]
  add   rdi, REQ_OFF_URL
  mov   rsi, [rsp]
  add   rsi, REQ_OFF_METHOD
  call  check_route
  cmp   rax, 0
  jl    .not_found

  mov   qword [response_status_code], OK
  mov   rax, 0

  jmp   .return

.method_not_allowed:
  mov   qword [response_status_code], METHOD_NOT_ALLOWED

  mov   rdi, [rsp]
  add   rdi, REQ_OFF_CLIENT
  lea   rsi, [STR_METHOD_NOT_ALLOWED]
  call  send_error_response

  mov   rax, FAILURE_CODE

  jmp   .return

.not_found:
  mov   qword [response_status_code], NOT_FOUND

  mov   rdi, [rsp]
  add   rdi, REQ_OFF_CLIENT
  lea   rsi, [STR_NOT_FOUND]
  call  send_error_response

  mov   rax, FAILURE_CODE

  jmp   .return

.return:
  add   rsp, 0x8
  ret

; TODO: pass request struct as arg
get_body:
  ; rdi -> request
  lea   rax, [request_body]
  ret

; checks if the method is allowed and returns its position in methods list
; @param  rdi: request method
; @return rax: method position in routes list
check_method:
  sub   rsp, 0x20

  mov   rsi, [rdi]
  mov   [rsp], rsi

  mov   rdi, rsi
  call  strlen

  cmp   rax, 0
  jle   .method_not_allowed

  mov   qword [rsp+0x8], 0
  mov   qword [rsp+0x10], rax

  ; look for the method inside methods list
  xor   rdi, rdi
  lea   rdi, [methods_list]
  mov   rsi, [rsp]
  call  strpos

  cmp   rax, 0
  jge   .method_exists
  
.method_not_allowed:
  mov   rax, FAILURE_CODE
  jmp   .return

.method_exists:
  ; store method's position onto the stack
  mov   qword [rsp+0x8], rax

  ; go to the method
  xor   rdi, rdi
  lea   rdi, [methods_list]
  add   rdi, qword [rsp+0x8]

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
  xor   rdi, rdi
  lea   rdi, [not_allowed_methods]
  mov   rsi, [rsp]
  call  strpos

  cmp   rax, 0
  jge   .method_not_allowed

  ; return method's position
  mov   rax, qword [rsp+0x8]

.return:
  add   rsp, 0x20
  ret

; checks if the route is available and returns its position in routes list
; @param  rdi: request url
; @param  rsi: request method
; @return rax: route position in routes list
check_route:
  sub   rsp, 0x20

  mov   rdx, [rdi]
  mov   [rsp], rdx

  mov   rdx, [rsi]
  mov   [rsp+0x8], rdx
  
  mov   qword [rsp+0x10], 0 ; route position inside list

  ; get url length
  mov   rdi, [rsp]
  call  strlen

  mov   rbx, rax

  ; remove query parameters, return url length
  mov   rsi, rbx
  call  trim_query_parameters

  cmp   rax, 0
  cmovg rbx, rax ; if rax > 0: found query param, exclude them

  mov   qword [rsp+0x18], rbx
  
  ; load url
  mov   rsi, [rsp]

  ; check if route is '/', if yes skip trim end slash
  cmp   qword [rsp+0x18], 1
  je    .skip_trim
  
  ; go to the end of the url
  add   rsi, qword [rsp+0x18] ; rsi points to the null_char

  ;  check if last char is slash
  dec   rsi
  cmp   byte [rsi], SLASH
  jne   .skip_trim
  
  ; if yes, change to null char and dec route length
  mov   byte [rsi], NULL_CHAR
  dec   qword [rsp+0x18]
  
.skip_trim:
  ; check if route is in routes_list
  xor   rbx, rbx
  lea   rdi, [routes_list]
  mov   rsi, [rsp]

.next_route:
  mov   rcx, qword [rsp+0x18]
  cld
  rep   cmpsb
  je    .route_found

.not_entire:
  add   rdi, STRUCT_ROUTE_LEN
  sub   rdi, qword [rsp+0x18]
  add   rdi, rcx
  add   rsi, rcx
  sub   rsi, qword [rsp+0x18]

  inc   rbx
  cmp   rbx, qword [routes_count]
  jg    .route_not_found

  jmp   .next_route
  
.route_not_found:
  mov   rax, FAILURE_CODE
  jmp   .return

.route_found:
  ; make sure we checked the entire route
  mov   al, byte [rdi]
  cmp   al, NULL_CHAR
  jne   .not_entire

  ; store position of the route
  xor   rdx, rdx
  mov   rax, STRUCT_ROUTE_LEN
  mul   rbx
  mov   qword [rsp+0x10], rax

  mov   rdi, [rsp+0x8]
  call  strlen

  ; check the method is correct
  lea   rdi, [routes_list]
  add   rdi, qword [rsp+0x10]
  add   rdi, URL_MAX_LEN
  mov   rsi, [rsp+0x8] 
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
  mov   rax, qword [rsp+0x18]

.return:
  add   rsp, 0x20
  ret

; extracts the value of a specified query parameter from a given URL string
; @param  rdi: query parameter
; @return rax: pointer to value
get_param:
  ; init stack
  sub   rsp, 0x10

  cmp   rdi, 0
  jle   .error

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

; frees the request structrure and all malloc'd variable
; @param  rdi: pointer to request struct
; @return rax: return code
free_request:
  sub   rsp, 0x8

  mov   [rsp], rdi

  mov   rsi, [rsp]
  add   rsi, REQ_OFF_METHOD
  mov   rdi, [rsi]
  call  free

  mov   rsi, [rsp]
  add   rsi, REQ_OFF_URL
  mov   rdi, [rsi]
  call  free

  mov   rsi, [rsp]
  add   rsi, REQ_OFF_PROTO
  mov   rdi, [rsi]
  call  free

  mov   rsi, [rsp]
  add   rsi, REQ_OFF_HEADERS
  mov   rdi, [rsi]
  call  free

  mov   rsi, [rsp]
  add   rsi, REQ_OFF_BODY
  mov   rdi, [rsi]
  test  rdi, rdi
  jz    .free_request
  call  free

.free_request:
  mov   rdi, [rsp]
  call  free
  
.return:
  add   rsp, 0x8
  ret


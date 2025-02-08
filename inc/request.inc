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
    req_str         dq 0
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
  REQ_OFF_STR         equ REQ_OFF_HOST + 0x8
  REQ_OFF_CLOSE       equ REQ_OFF_STR + 0x8
  
section .text
; sets the string represenation of the request
; @param  rdi: pointer to request struct
; @param  rsi: request str
; @return rax: return code
set_request_str:
  cmp   rdi, 0
  jl    .error
  
  cmp   rsi, 0
  jl    .error

  mov   qword [rdi+REQ_OFF_STR], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the length of the request length
; @param  rdi: pointer to request struct
; @param  rsi: request length
; @return rax: return code
set_request_length:
  cmp   rdi, 0
  jl    .error
  
  cmp   rsi, 0
  jl    .error

  mov   qword [rdi+REQ_OFF_LEN], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; receives the request from the client and put it into the request struct
; @param  rdi: pointer to the context struct
; @param  rax: return code
receive_request:
  sub   rsp, 0x18

  mov   [rsp], rdi

  ; malloc request str  
  mov   rdi, REQUEST_MAX_LEN
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax 

  ; get request struct
  mov   rdi, [rsp]
  call  get_ctx_request

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; get client
  mov   rdi, [rsp]
  call  get_ctx_client

  cmp   rax, 0
  jl    .error

  ; get client fd
  mov   rdi, rax
  call  get_client_fd

  cmp   rax, 0
  jl    .error

  mov   rdi, rax

  ; receive client request
  mov   rax, SYS_RECVFROM
  mov   rsi, [rsp+0x8]
  mov   rdx, REQUEST_MAX_LEN
  xor   r10, r10
  xor   r9, r9
  xor   r8, r8
  syscall

  cmp   rax, 0
  jl    .error

  ; save request length
  mov   rdi, [rsp+0x10]
  mov   rsi, rax
  call  set_request_length

  cmp   rax, 0
  jl    .error

  ; save request str
  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp+0x8]
  call  set_request_str

  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE
  
.return:
  add   rsp, 0x18
  ret


; parses the string representation of the request onto the request struct
; and saves it inside the context
; @param  rdi: pointer to context struct
; @return rax: return code
parse_request:
  sub   rsp, 0x20

  cmp   rdi, 0
  jle   .error

  mov   [rsp], rdi

  ; get request struct
  call  get_ctx_request

  cmp   rax, 0
  jl    .error

  ; save pointer of the request struct
  mov   [rsp+0x8], rax
  
  ; get request str
  mov   rdi, rax
  call  get_request_str

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; extract method: look for the next space
  mov   rdi, [rsp+0x10]
  mov   rsi, SPACE
  call  find_next_char

  ; if < 0, bad request
  cmp   rax, 0
  jl    .error 

  ; compare the length of the method with METHOD_MAX_LEN
  cmp   rax, METHOD_MAX_LEN
  ; if bigger, send bad request
  jg    .error

  mov   qword [rsp+0x18], rax

  ; add one for null char
  mov   rdi, rax
  inc   rdi
  call  malloc

  cmp   rax, 0
  jle   .error

  mov   rdi, [rsp+0x8]
  mov   [rdi+REQ_OFF_METHOD], rax

  ; load method
  mov   rdi, rax
  mov   rcx, qword [rsp+0x18]
  mov   rsi, [rsp+0x10] 
  rep   movsb

  xor   rax, rax
  stosb

  ; method url HTTP/n.n 
  ;       | inc rsi to go to first char of the url
  inc   rsi

  ; save rsi pointer to the stack
  mov   [rsp+0x10], rsi

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

  mov   qword [rsp+0x18], rax

  ; add one for null char
  mov   rdi, rax
  inc   rdi
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   [rdi+REQ_OFF_URL], rax

  ; load url
  mov   rdi, rax
  mov   rcx, qword [rsp+0x18]
  mov   rsi, [rsp+0x10]
  rep   movsb

  xor   rax, rax
  stosb

  ; inc rsi to go to the protocol 
  inc   rsi

  ; extract proto
  ; save rsi pointer to the stack
  mov   [rsp+0x10], rsi

  xor   rdi, rdi
  lea   rdi, [rsi]
  mov   rsi, CARRIAGE_RETURN
  call  find_next_char  ; find next \r to get the entire protocol
  cmp   rax, 0x0
  jl    .error

  mov   qword [rsp+0x18],rax

  ; malloc request protocol
  mov   rdi, PROTO_MAX_LEN
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   [rdi+REQ_OFF_PROTO], rax

  ; load protocol
  mov   rdi, rax
  mov   rcx, qword [rsp+0x18]
  mov   rsi, [rsp+0x10]
  rep   movsb

  ; add null char to avoid garbage values
  xor   rax, rax
  stosb

  ; save rsi pointer to the stack
  mov   [rsp+0x10], rsi

  ; mov to new line (skip 2 char \r\n)
  add   rsi, 0x2

  ; save headers onto the stack
  mov   [rsp+0x10], rsi

  ; find the end of the headers (\r\n\r\n)
  lea   rdi, [rsi]
  lea   rsi, [post_headers]
  call  strpos

  cmp   rax, 0
  jl    .error

  ; verify headers overflow
  cmp   rax, HEADERS_MAX_LEN
  jg    .error

  mov   qword [rsp+0x18], rax
   
  mov   rdi, rax
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   [rdi+REQ_OFF_HEADERS], rax

  mov   rdi, rax
  mov   rcx, qword [rsp+0x18]
  mov   rsi, [rsp+0x10]
  rep   movsb

  ; get body
  ; move to new line (skip 4 char \r\n\r\n)
  add   rsi, 0x4

  mov   [rsp+0x10], rsi

  ; find the end of the request
  lea   rdi, [rsi]
  mov   rsi, NULL_CHAR
  call  find_next_char

  cmp   rax, 0
  jl    .error
  je    .no_body

  cmp   rax, BODY_MAX_LEN
  jg    .error

  mov   qword [rsp+0x18], rax
  mov   rdi, [rsp+0x8]
  mov   [rdi+REQ_OFF_CONTENT_LEN], rax

  mov   rdi, rax
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   [rdi+REQ_OFF_BODY], rax

  mov   rdi, rax
  mov   rcx, qword [rsp+0x18]
  mov   rsi, [rsp+0x10]
  rep   movsb

  mov   rax, [rsp+0x8]

  jmp   .return

.no_body:
  mov   rdi, [rsp+0x8]
  mov   qword [rdi+REQ_OFF_BODY], 0

  mov   rax, [rsp+0x8]
  jmp   .return

.error:
  mov   qword [response_status_code], BAD_REQUEST
  mov   rdi, qword [rsp+0x8]
  lea   rsi, [STR_BAD_REQUEST]
  call  send_error_response

  mov   rax, FAILURE_CODE
  jmp   .return

.return:
  add   rsp, 0x20
  ret

; verifies the request, making sure the route exists and is allowed
; @param  rdi: pointer to server struct
; @param  rsi: pointer to request struct
; @return rax: pointer to route
verify_request:
  sub   rsp, 0x20

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  
  call  get_server_routes 

  mov   rdi, INTERNAL_SERVER_ERROR
  cmp   rax, 0
  jl    .error

  ; save routes onto the stack
  mov   [rsp+0x10], rax

  mov   rsi, rax

.loop:
  ; save route onto the stack
  mov   [rsp+0x18], rsi

  mov   rdi, [rsp+0x8]
  call  compare_route

  cmp   rax, 0
  jl    .error
  jg    .end_loop ; route is valid

  ; go to next route
  mov   rdx, [rsp+0x18]
  mov   rsi, [rdx+ROUTE_OFF_NEXT]

  cmp   rsi, 0
  je    .error

  jmp   .loop
.end_loop:
  
  mov   rax, [rsp+0x18]
  jmp   .return

.error:
  ; in the future, need to set a specific code when I have context struct
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret

; returns the body of a request
; @param  rdi: pointer to request struct
; @return rax: pointer to request's body
get_body:
  add   rdi, REQ_OFF_BODY
  mov   rax, [rdi]
  ret

; returns the headers of a request
; @param  rdi: pointer to request struct
; @return rax: pointer to request's header
get_request_headers:
  add   rdi, REQ_OFF_HEADERS
  mov   rax, [rdi]
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

; returns the pointer to the string representation of the request
; @param  rdi: pointer to request
; @return rax: pointer to string
get_request_str:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+REQ_OFF_STR]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret


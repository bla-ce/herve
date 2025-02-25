section .data
  response_struct:
    res_str         dq 0
    res_client      dq 0    
    res_len         dq 0
    res_proto       dq 0
    res_headers     dq 0
    res_body        dq 0
    res_content_len dq 0
    res_status_code dq 0
  end_response_struct:

  RESPONSE_STRUCT_LEN equ end_response_struct - response_struct

  RESPONSE_MAX_LEN equ 0x8000

  ; offsets
  RESPONSE_OFF_STR              equ 0x0
  RESPONSE_OFF_CLIENT           equ RESPONSE_OFF_STR + 0x8
  RESPONSE_OFF_LEN              equ RESPONSE_OFF_CLIENT + 0x8
  RESPONSE_OFF_PROTO            equ RESPONSE_OFF_LEN + 0x8
  RESPONSE_OFF_HEADERS          equ RESPONSE_OFF_PROTO + 0x8
  RESPONSE_OFF_BODY             equ RESPONSE_OFF_HEADERS + 0x8
  RESPONSE_OFF_CONTENT_LEN_STR  equ RESPONSE_OFF_BODY + 0x8
  RESPONSE_OFF_STATUS_CODE      equ RESPONSE_OFF_CONTENT_LEN_STR + 0x8

  STR_CONTENT_LENGTH_MAX_LENGTH equ 7 ; add one for null char

  EMPTY_CONTENT db "0", NULL_CHAR

section .text
; frees the response structure and all associated resources
; @param  rdi: pointer to the response
; @return rax: return code
free_response:
  sub   rsp, 0x8

  cmp   rdi, 0
  jl    .error

  mov   [rsp], rdi

.free_struct:
  ; free response struct
  mov   rdi, [rsp]
  call  free

  cmp   rax, 0
  jl    .error

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret 

; sets the string representation of the response
; @param  rdi: pointer to response struct
; @param  rsi: pointer to the string
; @return rax: return code
set_response_str:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   [rdi+RESPONSE_OFF_STR], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the response string
; @param  rdi: pointer to response struct
; @return rax: pointer to the string
get_response_str:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+RESPONSE_OFF_STR]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the response content length string
; @param  rdi: pointer to response
; @return  rax: pointer to string
get_response_content_len_str:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+RESPONSE_OFF_CONTENT_LEN_STR]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the response content length string
; @param  rdi: pointer to response
; @param  rsi: pointer to string
; @return rax: return code
set_response_content_len_str:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   [rdi+RESPONSE_OFF_CONTENT_LEN_STR], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; TODO: read ctx protocol, return HTTP/1.1 for now
; returns the response protocol 
; @param  rdi: pointer to response
; @param  rax: pointer to response headers
get_response_protocol:
  lea   rax, [HTTP_1_1]
  ret

; returns the response headers 
; @param  rdi: pointer to response
; @param  rax: pointer to response headers
get_response_headers:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+RESPONSE_OFF_HEADERS]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the response status code 
; @param  rdi: pointer to response
; @param  rax: pointer to response status code
get_response_status_code:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+RESPONSE_OFF_STATUS_CODE]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; initialise the response struct and malloc necessary resources
; @return rax: pointer to response struct
response_init:
  sub   rsp, 0x8

  mov   qword [rsp], 0

  ; malloc response
  mov   rdi, RESPONSE_STRUCT_LEN
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   [rsp], rax

  ; create hash table for headers
  mov   rdi, HEADERS_MAX_COUNT
  call  ht_create

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, rax
  call  set_response_headers

  cmp   rax, 0
  jl    .error

  ; set body to 0
  mov   rdi, [rsp]
  xor   rsi, rsi
  call  set_response_body

  cmp   rax, 0
  jl    .error

  mov   rax, [rsp]
  jmp   .return

.error:
  mov   rax, qword [rsp]
  test  rax, rax
  jz    .no_free

  mov   rdi, rax
  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; sets the headers of the response struct
; @param  rdi: pointer to the response struct
; @param  rsi: pointer to the headers array
; @return rax: return code
set_response_headers:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   [rdi+RESPONSE_OFF_HEADERS], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the status code of the response struct
; @param  rdi: pointer to the response struct
; @param  rsi: pointer to the string status code
; @return rax: return code
set_response_status_code:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   [rdi+RESPONSE_OFF_STATUS_CODE], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the body of the response
; @param  rdi: pointer to the response struct
; @param  rsi: pointer to body
; @return rax: return code
set_response_body:
  cmp   rdi, 0
  jle   .error

  ; body can be 0

  mov   [rdi+RESPONSE_OFF_BODY], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the body of the response
; @param  rdi: pointer to the response struct
; @return rax: pointer to the body (can be null)
get_response_body:
  cmp   rdi, 0
  jle   .error

  ; check if body is 0
  mov   rax, [rdi+RESPONSE_OFF_BODY]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; stringifies the response struct
; @param  rdi: pointer to response struct
; @return rax: pointer to string representation of the response
stringify_response:
  sub   rsp, 0x40

  mov   [rsp], rdi

  ; get context protocol
  call  get_response_protocol

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax
  mov   qword [rsp+0x28], 0

  ; get status code
  mov   rdi, [rsp]
  call  get_response_status_code

  cmp   rax, 0
  jl    .error
   
  ; get string status code
  mov   rdi, rax
  call  get_status_string

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; get headers
  mov   rdi, [rsp]
  call  get_response_headers
  
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; get body
  mov   rdi, [rsp]
  call  get_response_body

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  ; TODO: get length before?
  ; malloc response string
  mov   rdi, RESPONSE_MAX_LEN
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x28], rax ; dynamic pointer
  mov   [rsp+0x30], rax ; origin

  ; append protocol
  mov   rdi, [rsp+0x8]
  call  strlen

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x28]
  mov   rsi, [rsp+0x8]
  mov   rcx, rax
  rep   movsb

  ; add space -> no need to increment
  mov   rax, SPACE
  stosb

  ; save rdi's position
  mov   [rsp+0x28], rdi

  ; append status code
  mov   rdi, [rsp+0x10]
  call  strlen

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x28]
  mov   rsi, [rsp+0x10]
  mov   rcx, rax
  rep   movsb

  ; add new line
  mov   rax, CARRIAGE_RETURN
  stosb

  mov   rax, LINE_FEED
  stosb

  mov   [rsp+0x28], rdi

  ; append headers
  ; stringify headers
  mov   rdi, [rsp+0x18]
  cmp   rdi, 0
  je    .skip_headers
  call  stringify_headers

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x38], rax

  mov   rdi, rax
  call  strlen

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x28]
  mov   rsi, [rsp+0x38]
  mov   rcx, rax
  rep   movsb

  mov   [rsp+0x28], rdi

  ; free headers string
  mov   rdi, [rsp+0x38]
  call  free
  cmp   rax, 0
  jl    .error

  ; free headers hash table
  mov   rdi, [rsp+0x18]
  call  ht_free
  cmp   rax, 0
  jl    .error

  ; check if body
.skip_headers:
  mov   rdi, [rsp+0x20]
  cmp   rdi, 0
  je    .skip_body

  ; append body
  call  strlen

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x28]
  mov   rsi, [rsp+0x20]
  mov   rcx, rax
  rep   movsb

  mov   [rsp+0x28], rdi

.skip_body:
  mov   rdi, [rsp+0x28]
  ; add null_char
  mov   rax, NULL_CHAR
  stosb

  ; save response str
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x30]
  call  set_response_str

  cmp   rax, 0
  jl    .error

  ; return response
  mov   rax, [rsp+0x30]

  jmp   .return

.error:
  mov   rax, [rsp+0x28]
  test  rax, rax
  jz    .no_free

  mov   rdi, rax
  call  free

  mov   rax, [rsp+0x38]
  test  rax, rax
  jz    .no_free

  mov   rdi, rax
  call  free

  mov   rax, [rsp+0x18]
  test  rax, rax
  jz    .no_free

  mov   rdi, rax
  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x40
  ret

; sends a response without content
; @param  rdi: pointer to context struct
; @param  rsi: status code
; @return rax: return code
send_no_content:
  sub   rsp, 0x10

  cmp   rdi, 0
  jl    .error

  ; default status code is 204 No Content
  mov   rax, NO_CONTENT
  cmp   rsi, 0
  cmove rsi, rax

  mov   [rsp], rdi
  mov   qword [rsp+0x8], rsi

  ; get response
  call  get_ctx_response
  cmp   rax, 0
  jl    .error

  ; get response headers
  mov   rdi, rax
  call  get_response_headers

  cmp   rax, 0
  jl    .error

  ; set content length to 0
  mov   rdi, rax
  mov   rsi, CONTENT_LENGTH_HEADER
  lea   rdx, [EMPTY_CONTENT]
  call  set_header

  cmp   rax, 0
  jl    .error

  ; send response
  mov   rdi, [rsp]
  mov   rsi, qword [rsp+0x8]
  call  send_response

  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; sends a string response
; @param  rdi: pointer to context struct
; @param  rsi: status code
; @param  rdx: pointer to string
; @return rax: return code
send_string:
  sub   rsp, 0x38

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  cmp   rdx, 0
  jle   .error

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   qword [rsp+0x30], 0

  ; get response
  call  get_ctx_response
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  mov   rdi, rax
  call  get_response_headers

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax
  
  ; set content type headers
  mov   rdi, [rsp+0x20]
  lea   rsi, [CONTENT_TYPE_HEADER]
  lea   rdx, [CONTENT_PLAIN]
  call  set_header

  cmp   rax, 0
  jl    .error
 
  ; get string length   
  mov   rdi, [rsp+0x10]
  call  strlen

  cmp   rax, 0
  jl    .error

  mov   qword [rsp+0x28], rax

  mov   rdi, LONG_SIZE
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x30], rax

  mov   rdi, qword [rsp+0x28]
  mov   rsi, [rsp+0x30]
  mov   rdx, LONG_SIZE
  call  to_string

  cmp   rax, 0
  jl    .error

  ; set content length
  mov   rdx, rax
  mov   rdi, [rsp+0x20]
  lea   rsi, [CONTENT_LENGTH_HEADER]
  call  set_header

  cmp   rax, 0
  jl    .error

  ; add string to body
  mov   rdi, [rsp+0x18]
  mov   rsi, [rsp+0x10]
  call  set_response_body

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  call  send_response

  cmp   rax, 0
  jl    .error

  ; free content length str
  mov   rdi, [rsp+0x30]
  call  free

  cmp   rax, 0
  jl    .error
  
  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, [rsp+0x30]
  test  rax, rax
  jz    .no_free

  mov   rdi, rax
  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x38
  ret

; sends response to client fd
; @param  rdi: pointer to context struct
; @param  rsi: status code (int)
; @return rax: return code
send_response:
  sub   rsp, 0x30

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x18], 0

  ; get response
  call  get_ctx_response
  cmp   rax, 0
  jl    .error
  
  mov   [rsp+0x10], rax

  ; set connection close header TODO: use variable from context
  mov   rdi, [rsp+0x10]
  call  get_response_headers

  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  lea   rsi, [CONNECTION_HEADER]
  lea   rdx, [CONNECTION_CLOSE]
  call  set_header

  cmp   rax, 0
  jl    .error

  ; set response status code
  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp+0x8]
  call  set_response_status_code

  ; stringify response
  mov   rdi, [rsp+0x10]
  call  stringify_response

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; get response length
  mov   rdi, rax
  call  strlen

  cmp   rax, 0
  jl    .error

  mov   qword [rsp+0x20], rax

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

  ; write response
  mov   rdi, rax
  mov   rax, SYS_WRITE
  mov   rsi, [rsp+0x18]
  mov   rdx, qword [rsp+0x20]
  syscall

  cmp   rax, 0
  jl    .error

  ; free response
  mov   rdi, [rsp+0x18]
  call  free

  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  ; make sure you free the response
  mov   rax, qword [rsp+0x18]
  test  rax, rax
  jz    .no_free

  mov   rdi, rax
  call  free
  
.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x30
  ret  

; TODO: make sure we have permissions
; serves a static file
; @param  rdi: ctx
; @param  rsi: file path
; @return rax: void
send_static_file:
  sub   rsp, 0x48

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x40], 0

  ; get file ext
  mov   rdi, rsi
  call  get_content_type

  cmp   rax, 0
  jl    .error_open

  mov   [rsp+0x10], rax ; save content type header

  ; get context response
  mov   rdi, [rsp]
  call  get_ctx_response

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; get response headers
  mov   rdi, [rsp+0x18]
  call  get_response_headers

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  ; set content type header
  mov   rdi, [rsp+0x20]
  lea   rsi, [CONTENT_TYPE_HEADER]
  mov   rdx, [rsp+0x10]
  call  set_header

  cmp   rax, 0
  jl    .error
 
  ; open file
  mov   rdi, [rsp+0x8]
  mov   rsi, O_RDONLY
  mov   rdx, 0
  call  open_file

  cmp   rax, 0
  jl    .error_open

  mov   qword [rsp+0x28], rax

  ; read file
  mov   rdi, qword [rsp+0x28]
  call  read_file

  cmp   rax, 0
  jl    .error_read

  mov   [rsp+0x30], rax

  ; get content length   
  mov   rdi, [rsp+0x30]
  call  strlen

  cmp   rax, 0
  jl    .error
  
  mov   qword [rsp+0x38], rax

  mov   rdi, LONG_SIZE
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x40], rax

  mov   rdi, qword [rsp+0x38]
  mov   rsi, [rsp+0x40]
  mov   rdx, LONG_SIZE
  call  to_string

  cmp   rax, 0
  jl    .error

  ; set content length
  mov   rdx, rax
  mov   rdi, [rsp+0x20]
  lea   rsi, [CONTENT_LENGTH_HEADER]
  call  set_header

  cmp   rax, 0
  jl    .error

  ; add string to body
  mov   rdi, [rsp+0x18]
  mov   rsi, [rsp+0x30]
  call  set_response_body

  cmp   rax, 0
  jl    .error

  ; send response
  mov   rdi, [rsp]
  mov   rsi, OK
  call  send_response

  cmp   rax, 0
  jl    .error

  ; free file content
  mov   rdi, [rsp+0x40]
  call  free

  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  ; clean stack
  jmp   .return

.error_open:
  ; load 404

.error_read:
  ; load 500

.error:
  mov   rax, [rsp+0x40]
  test  rax, rax
  jz    .no_free

  mov   rdi, rax
  call  free
  
.no_free:
  mov   rax, FAILURE_CODE

.return:
  ; clean stack
  add   rsp, 0x48
  ret

; serve file from directory
; @param  rdi: pointer to context struct
serve_dir:
  sub   rsp, 0x8

  cmp   rdi, 0
  jle   .error

  mov   [rsp], rdi

  ; get context request
  mov   rdi, [rsp]
  call  get_ctx_request
  
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  get_request_url

  cmp   rax, 0
  jl    .error

  ; request url starts with '/', remove it
  inc   rax

  mov   rdi, [rsp]
  mov   rsi, rax
  call  send_static_file

  cmp   rax, 0
  jl    .error

  jmp   .return

.error:
  ; send 500
  mov   rdi, [rsp] 
  mov   rsi, INTERNAL_SERVER_ERROR
  call  send_response  

  mov   rdi, [rsp]
  call  ctx_abort

  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret


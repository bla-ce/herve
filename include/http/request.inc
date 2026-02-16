section .bss
request_t:
  .client       resq 1
  .len          resq 1
  .method       resq 1
  .url          resq 1
  .proto        resq 1
  .headers      resq 1
  .body         resq 1
  .content_len  resq 1
  .host         resq 1
  .str          resq 1
  .query_param  resq 1
  .dyn_param    resq 1
  .close        resq 1
request_t_end:

section .data
  ; constants
  METHOD_MAX_LEN  equ 0x8
  PROTO_MAX_LEN   equ 0x9
  HOST_MAX_LEN    equ 0xFF
  URL_MAX_LEN     equ 0x2000
  BODY_MAX_LEN    equ 0x2000
  REQUEST_MAX_LEN equ 0x2000

  REQ_STRUCT_LEN equ request_t_end - request_t

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
  REQ_OFF_QUERY_PARAM equ REQ_OFF_STR + 0x8
  REQ_OFF_DYN_PARAM   equ REQ_OFF_QUERY_PARAM + 0x8
  REQ_OFF_CLOSE       equ REQ_OFF_DYN_PARAM + 0x8

section .text
; initialise the request with null pointers
; @param  rdi: pointer to the request struct
; @return rax: return value
request_init:
  sub   rsp, 0x8

  cmp   rdi, 0
  jl    .error

  mov   [rsp], rdi

  mov   rdi, [rsp]
  xor   rsi, rsi
  mov   rdx, REQ_STRUCT_LEN
  call  memset

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; receives the request from the client and parses it
; @param  rdi: pointer to the context struct
; @return rax: return code
handle_request:
  sub   rsp, 0x10
  sub   rsp, REQUEST_MAX_LEN

  cmp   rdi, 0
  jle   .error

  ; STACK USAGE
  ; [rsp]     -> pointer to the context struct
  ; [rsp+0x8] -> request string

  mov   [rsp], rdi

  mov   rdi, [rsp]
  lea   rsi, [rsp+0x8]
  call  receive_request
  cmp   rax, 0
  jle   .error

  mov   rdi, [rsp]
  lea   rsi, [rsp+0x8]
  call  parse_request
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, REQUEST_MAX_LEN
  add   rsp, 0x10
  ret

; sets the url of the request
; @param  rdi: pointer to the request
; @param  rsi: pointer to the url
; @return rax: return code
set_request_url:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   [rdi+REQ_OFF_URL], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the method of the request
; @param  rdi: pointer to the request
; @param  rsi: pointer to the method
; @return rax: return code
set_request_method:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   [rdi+REQ_OFF_METHOD], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the protocol of the request
; @param  rdi: pointer to the request
; @param  rsi: pointer to the protocol
; @return rax: return code
set_request_proto:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   [rdi+REQ_OFF_PROTO], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the headers of the request
; @param  rdi: pointer to the request
; @param  rsi: pointer to the headers
; @return rax: return code
set_request_headers:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   [rdi+REQ_OFF_HEADERS], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the body of the request
; @param  rdi: pointer to the request
; @param  rsi: pointer to the body
; @return rax: return code
set_request_body:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   [rdi+REQ_OFF_BODY], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; parses the form data contained inside the body to a hash table
; @param  rdi: pointer to the request struct
; @return rax: pointer to the hash table
parse_form_data:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp] -> pointer to the request struct

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

  ; get headers of the request
  mov   rdi, [rsp]
  call  get_request_headers
  cmp   rax, 0
  jl    .error

  ; make sure form encoded header is present
  ; TODO: fix this
  mov   rdi, rax
  mov   rsi, CONTENT_TYPE_HEADER
  call  get_header_value
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, FORM_ENCODED
  call  strcmp
  cmp   rax, 0
  jl    .error
  cmp   rax, TRUE
  jne   .error

  ; get body of the request
  mov   rdi, [rsp]
  call  get_request_body
  cmp   rax, 0
  jl    .error

  ; parses form data
  mov   rdi, rax
  call  parse_query_param
  cmp   rax, 0
  jl    .error

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; parses query parameters into a hash table
; @param  rdi: pointer to encoded data
; @return rax: pointer to the hash table
parse_query_param:
  sub   rsp, 0x30

  ; STACK USAGE
  ; [rsp]       -> pointer to the encoded data
  ; [rsp+0x8]   -> dynamic pointer
  ; [rsp+0x10]  -> number of parameter
  ; [rsp+0x18]  -> pointer to the hash table
  ; [rsp+0x20]  -> pointer to the key
  ; [rsp+0x28]  -> pointer to the value

  mov   [rsp], rdi
  mov   [rsp+0x8], rdi
  mov   qword [rsp+0x18], 0
  mov   qword [rsp+0x20], 0
  mov   qword [rsp+0x28], 0

  cmp   rdi, 0
  jle   .error

  ; rdi now points to the beginning of the query param
  ; get the number of '&' to get the number of keys
  mov   rdi, [rsp+0x8]
  mov   rsi, AMPERSAND
  call  count_char
  cmp   rax, 0
  jl    .error

  inc   rax ; add one (for the first key)

  mov   qword [rsp+0x10], rax

  ; create hash table
  mov   rdi, qword [rsp+0x10]
  call  ht_create
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; load url at the query pos
  mov   rdi, [rsp+0x8]

.loop:
  mov   rdi, [rsp+0x8]

  ; look for '='
  mov   rsi, EQUAL_SIGN
  call  find_next_char
  cmp   rax, 0
  jl    .loop_end

  mov   rsi, [rsp+0x8]
  mov   rdi, rsi

  add   rsi, rax
  inc   rsi             ; add one to go after the '='
  mov   [rsp+0x8], rsi

  ; strndup key
  mov   rsi, rax
  call  strndup
  cmp   rax, 0
  jl    .error

  ; save key
  mov   [rsp+0x20], rax

  ; get next ampersand
  mov   rdi, [rsp+0x8]
  mov   rsi, AMPERSAND
  call  find_next_char
  cmp   rax, 0
  jl    .get_last_key

  mov   rdi, [rsp+0x8]
  mov   rsi, rdi
  add   rsi, rax
  inc   rsi       ; add one to go after '&'
  mov   [rsp+0x8], rsi

  ; strndup value
  mov   rsi, rax
  call  strndup
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x28], rax

.add_key_value:
  mov   rdi, [rsp+0x18]
  mov   rsi, [rsp+0x20]
  mov   rdx, [rsp+0x28]
  call  ht_insert
  cmp   rax, 0
  jl    .error

  ; free key
  mov   rdi, [rsp+0x20]
  call  free
  cmp   rax, 0
  jl    .error

  ; free value
  mov   rdi, [rsp+0x28]
  call  free
  cmp   rax, 0
  jl    .error

  ; go to next
  jmp   .loop

  ; check if last query
.get_last_key:
  mov   rdi, [rsp+0x8]
  call  strlen
  cmp   rax, 0
  jle   .error

  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  strndup
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x28], rax

  mov   rdi, [rsp+0x18]
  mov   rsi, [rsp+0x20]
  mov   rdx, [rsp+0x28]
  call  ht_insert
  cmp   rax, 0
  jl    .error

  ; free key
  mov   rdi, [rsp+0x20]
  call  free
  cmp   rax, 0
  jl    .error

  ; free value
  mov   rdi, [rsp+0x28]
  call  free
  cmp   rax, 0
  jl    .error

.loop_end:
  mov   rax, [rsp+0x18]

  jmp   .return

.no_query:
  xor   rax, rax
  jmp   .return

.error:
  ; TODO: return 500

  mov   rdi, [rsp+0x18]
  test  rdi, rdi
  jz    .no_ht_free

  call  ht_free

.no_ht_free:
  mov   rdi, [rsp+0x20]
  test  rdi, rdi
  jz    .no_key_free

  call  free

.no_key_free:
  mov   rdi, [rsp+0x28]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x30
  ret

; returns query parameter
; @param  rdi: pointer to request struct
; @param  rsi: key
; @return rax: value
get_query_param:
  sub   rsp, 0x10

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; get query hash table
  mov   rdi, [rsp]
  call  get_request_query_param
  cmp   rax, 0
  jle   .error

  mov   rdi, rax
  mov   rsi, [rsp+0x8]
  call  ht_get
  cmp   rax, 0
  jle   .error

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; returns the method of the request
; @param  rdi: pointer to request struct
; @return rax: pointer to url
get_request_method:
  cmp   rdi, 0
  jl    .error

  mov   rax, qword [rdi+REQ_OFF_METHOD]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the url of the request
; @param  rdi: pointer to request struct
; @return rax: pointer to url
get_request_url:
  cmp   rdi, 0
  jl    .error

  mov   rax, qword [rdi+REQ_OFF_URL]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

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

; returns the dynamic parameters of the request
; @param  rdi: pointer to request struct
; @return rax: pointer to the hash table
get_request_dynamic_param:
  cmp   rdi, 0
  jl    .error

  mov   rax, qword [rdi+REQ_OFF_DYN_PARAM]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the dynamic parameters of the request
; @param  rdi: pointer to request struct
; @param  rsi: pointer to the hash table
; @return rax: return code
set_request_dynamic_param:
  cmp   rdi, 0
  jl    .error

  cmp   rsi, 0
  jl    .error

  mov   qword [rdi+REQ_OFF_DYN_PARAM], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the query parameters of the request
; @param  rdi: pointer to request struct
; @return rax: pointer to the hash table
get_request_query_param:
  cmp   rdi, 0
  jl    .error

  mov   rax, qword [rdi+REQ_OFF_QUERY_PARAM]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the query parameters of the request
; @param  rdi: pointer to request struct
; @param  rsi: pointer to the hash table
; @return rax: return code
set_request_query_param:
  cmp   rdi, 0
  jl    .error

  cmp   rsi, 0
  jl    .error

  mov   qword [rdi+REQ_OFF_QUERY_PARAM], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the length of the request
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

; gets the length of the request
; @param  rdi: pointer to request struct
; @return rax: request length
get_request_length:
  cmp   rdi, 0
  jl    .error

  mov   rax, qword [rdi+REQ_OFF_LEN]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; receives the request from the client and put it into the request struct
; @param  rdi: pointer to the context struct
; @param  rsi: pointer to the buffer that will contain the request string
; @param  rax: size of the request
receive_request:
  sub   rsp, 0x20

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  ; get request struct
  mov   rdi, [rsp]
  call  ctx_get_request
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; get client
  mov   rdi, [rsp]
  call  ctx_get_client
  cmp   rax, 0
  jl    .error

  ; get client fd
  mov   rdi, rax
  call  client_get_fd
  cmp   rax, 0
  jl    .error

  ; receive client request
  mov   rdi, rax
  mov   rsi, [rsp+0x8]
  mov   rdx, REQUEST_MAX_LEN
  call  receive_from_file
  cmp   rax, 0
  jl    .error
  je    .empty

  mov   qword [rsp+0x18], rax

  ; save request length
  mov   rdi, [rsp+0x10]
  mov   rsi, rax
  call  set_request_length
  cmp   rax, 0
  jl    .error

  ; add null char at the end of the request to avoid garbage
  mov   rdi, [rsp+0x8]
  mov   rax, qword [rsp+0x18]
  add   rdi, rax
  mov   rax, NULL_CHAR
  stosb

  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp+0x8]
  call  set_request_str
  cmp   rax, 0
  jl    .error

  mov   rax, qword [rsp+0x18]

  jmp   .return

.empty:
  mov   rdi, [rsp]
  call  ctx_abort

  mov   rax, 0
  jmp   .return

.error:
  mov   rdi, [rsp]
  call  ctx_abort

  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret

; parses the string representation of the request onto the request struct
; and saves it inside the context
; @param  rdi: pointer to context struct
; @param  rsi: pointer to the request string
; @return rax: return code
parse_request:
  sub   rsp, 0x30

  ; STACK USAGE
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> pointer to the request struct
  ; [rsp+0x10]  -> pointer to the request string
  ; [rsp+0x18]  -> offset to extract request info
  ; [rsp+0x20]  -> pointer to headers string
  ; [rsp+0x28]  -> pointer to request url

  mov   [rsp], rdi
  mov   [rsp+0x10], rsi

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; get request struct
  call  ctx_get_request
  cmp   rax, 0
  jl    .error

  ; save pointer of the request struct
  mov   [rsp+0x8], rax

  ; extract method: look for the next space
  mov   rdi, [rsp+0x10]
  mov   rsi, SPACE
  call  find_next_char
  cmp   rax, 0
  jl    .error

  ; compare the length of the method with METHOD_MAX_LEN
  cmp   rax, METHOD_MAX_LEN
  ; if bigger, send bad request
  jg    .error

  mov   qword [rsp+0x18], rax

  ; add one for null char
  mov   rdi, [rsp+0x10]
  mov   rsi, qword [rsp+0x18]
  call  strndup
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   [rdi+REQ_OFF_METHOD], rax

  ; load method
  mov   rsi, [rsp+0x10]
  add   rsi, qword [rsp+0x18]

  ; method url HTTP/n.n
  ;       | inc rsi to go to first char of the url
  inc   rsi

  ; save rsi pointer to the stack
  mov   [rsp+0x10], rsi

  ; extract url: look for the next space
  mov   rdi, rsi
  mov   rsi, SPACE
  call  find_next_char
  cmp   rax, 0
  jl    .error

  ; verify url length
  cmp   rax, URL_MAX_LEN
  jg    .error

  mov   qword [rsp+0x18], rax
  cmp   rax, 1
  je    .strndup

  ; check if url ends with '/', if yes, trim it
  mov   rsi, [rsp+0x10]
  add   rsi, qword [rsp+0x18]
  dec   rsi
  cmp   byte [rsi], SLASH
  jne   .strndup

  dec   qword [rsp+0x18]

.strndup:
  ; add one for null char
  mov   rdi, [rsp+0x10]
  mov   rsi, qword [rsp+0x18]
  call  strndup
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   [rdi+REQ_OFF_URL], rax

  ; load url
  mov   rsi, [rsp+0x10]
  add   rsi, qword [rsp+0x18]

  ; inc rsi to go to the protocol
  inc   rsi

  ; extract proto
  ; save rsi pointer to the stack
  mov   [rsp+0x10], rsi

  mov   rdi, rsi
  mov   rsi, CARRIAGE_RETURN
  call  find_next_char  ; find next \r to get the entire protocol
  cmp   rax, 0x0
  jl    .error

  mov   qword [rsp+0x18],rax

  ; malloc request protocol
  mov   rdi, [rsp+0x10]
  mov   rsi, qword [rsp+0x18]
  call  strndup
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   [rdi+REQ_OFF_PROTO], rax

  ; load protocol
  mov   rsi, [rsp+0x10]
  add   rsi, qword [rsp+0x18]

  ; mov to new line (skip 2 char \r\n)
  add   rsi, 0x2

  ; save headers onto the stack
  mov   [rsp+0x10], rsi

  ; find the end of the headers (\r\n\r\n)
  mov   rdi, rsi
  mov   rsi, CRLF
  call  strpos
  cmp   rax, 0
  jl    .error

  ; verify headers overflow
  cmp   rax, HEADERS_MAX_LEN
  jg    .error

  mov   qword [rsp+0x18], rax

  mov   rdi, [rsp+0x10]
  mov   rsi, qword [rsp+0x18]
  call  strndup
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  mov   rsi, [rsp+0x10]
  add   rsi, qword [rsp+0x18]

  ; get body
  ; move to new line (skip CRLF)
  add   rsi, 0x4

  mov   [rsp+0x10], rsi

  ; parse headers
  mov   rdi, [rsp+0x20]
  call  parse_headers
  cmp   rax, 0
  jl    .error

  ; save headers
  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  set_request_headers
  cmp   rax, 0
  jl    .error

  ; free headers string
  mov   rdi, [rsp+0x20]
  call  free
  cmp   rax, 0
  jl    .error

  ; find the end of the request
  mov   rdi, [rsp+0x10]
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

  mov   rdi, [rsp+0x10]
  mov   rsi, qword [rsp+0x18]
  call  strndup
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   [rdi+REQ_OFF_BODY], rax

  mov   rax, [rsp+0x8]

  jmp   .parse_query_param

.no_body:
  mov   rdi, [rsp+0x8]
  mov   qword [rdi+REQ_OFF_BODY], 0

.parse_query_param:
  ; get request url
  mov   rdi, [rsp+0x8]
  call  get_request_url
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x28], rax

  ; go after the '?'
  mov   rdi, [rsp+0x28]
  mov   rsi, QUESTION_MARK
  call  find_next_char
  cmp   rax, 0
  jle   .no_query

  mov   rdi, [rsp+0x28]
  add   rdi, rax
  inc   rdi       ; go after the '?'
  call  parse_query_param
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  set_request_query_param
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x8]
  jmp   .return

.no_query:
  mov   rax, [rsp+0x8]
  mov   qword [rax+REQ_OFF_QUERY_PARAM], 0

  jmp   .return

.error:
  ; send 500
  mov   rdi, [rsp]
  mov   rsi, INTERNAL_SERVER_ERROR
  call  send_no_content

  mov   rdi, [rsp]
  call  ctx_abort

  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x30
  ret

; verifies the request, making sure the route exists and is allowed
; @param  rdi: pointer to context struct
; @return rax: pointer to route
verify_request:
  sub   rsp, 0x28

  ; STACK USAGE
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> pointer to the server struct
  ; [rsp+0x10]  -> pointer to the array of routes
  ; [rsp+0x18]  -> pointer to the request struct
  ; [rsp+0x20]  -> dynamic pointer to the route

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

  ; get context server
  call  ctx_get_server
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; get routes server
  mov   rdi, rax
  call  get_server_routes
  cmp   rax, 0
  jl    .error
  je    .not_found

  ; save routes onto the stack
  mov   [rsp+0x10], rax

  ; get ctx request
  mov   rdi, [rsp]
  call  ctx_get_request
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; check if the host header is present in the request
  mov   rdi, [rsp+0x18]
  call  get_request_headers
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   rsi, HOST_HEADER
  call  ht_get
  cmp   rax, 0
  jle   .error

  mov   rsi, [rsp+0x10]

.loop:
  ; save route onto the stack
  mov   [rsp+0x20], rsi

  mov   rdi, [rsp+0x18]
  call  route_match_request
  cmp   rax, 0
  jl    .error
  jg    .loop_end ; route is valid

  ; go to next route in server ll
  mov   rdx, [rsp+0x20]
  mov   rsi, [rdx+ROUTE_OFF_SERVER_NEXT]

  cmp   rsi, 0
  je    .not_found

  jmp   .loop
.loop_end:
  ; check if the route is active
  mov   rdi, [rsp+0x20]
  cmp   qword [rdi+ROUTE_OFF_ACTIVE], TRUE
  jne   .not_found

  ; check if the group is active
  mov   rdi, [rsp+0x8]

  mov   rax, [rsp+0x20]
  jmp   .return

.not_found:
  ; check if there is a 404 handler
  mov   rdi, [rsp+0x8]
  mov   rax, [rdi+SERVER_OFF_404_HANDLER]
  test  rax, rax
  jz    .send_404

  mov   rdi, [rsp]
  call  rax

  mov   rdi, [rsp]
  call  ctx_abort
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.send_404:
  ; send 404
  mov   rdi, [rsp]
  mov   rsi, NOT_FOUND
  call  send_no_content

  mov   rdi, [rsp]
  call  ctx_abort
  cmp   rax, 0
  jl    .error

  jmp   .return

.error:
  ; send bad request
  mov   rdi, [rsp]
  mov   rsi, BAD_REQUEST
  call  send_no_content

  mov   rdi, [rsp]
  call  ctx_abort
  cmp   rax, 0
  jl    .error

  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x28
  ret

; frees the request structrure and all malloc'd variable
; @param  rdi: pointer to request struct
; @return rax: return code
free_request:
  sub   rsp, 0x8

  mov   [rsp], rdi

  ; free method
.free_method:
  mov   rdi, [rsp]
  call  get_request_method
  cmp   rax, 0
  jl    .error
  je    .free_url

  mov   rdi, rax
  call  free
  cmp   rax, 0
  jl    .error

.free_url:
  mov   rdi, [rsp]
  call  get_request_url
  cmp   rax, 0
  jl    .error
  je    .free_proto

  mov   rdi, rax
  call  free
  cmp   rax, 0
  jl    .error

.free_proto:
  mov   rdi, [rsp]
  call  get_request_proto
  cmp   rax, 0
  jl    .error
  je    .free_headers

  mov   rdi, rax
  call  free
  cmp   rax, 0
  jl    .error

.free_headers:
  mov   rdi, [rsp]
  call  get_request_headers
  cmp   rax, 0
  jl    .error
  je    .free_body

  mov   rdi, rax
  call  ht_free
  cmp   rax, 0
  jl    .error

.free_body:
  mov   rdi, [rsp]
  call  get_request_body
  cmp   rax, 0
  jl    .error
  je    .free_query_param

  mov   rdi, rax
  call  free
  cmp   rax, 0
  jl    .error

.free_query_param:
  mov   rdi, [rsp]
  call  get_request_query_param
  cmp   rax, 0
  jl    .error
  je    .free_dynamic_param

  mov   rdi, rax
  call  ht_free
  cmp   rax, 0
  jl    .error

.free_dynamic_param:
  mov   rdi, [rsp]
  call  get_request_dynamic_param
  cmp   rax, 0
  jl    .error
  je    .free_struct

  mov   rdi, rax
  call  ht_free
  cmp   rax, 0
  jl    .error

.free_struct:
  ; free request struct
  mov   rdi, [rsp]
  call  free

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

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

; returns the pointer to the protocol of the request
; @param  rdi: pointer to request
; @return rax: pointer to request protocol
get_request_proto:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+REQ_OFF_PROTO]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the pointer to the headers of the request
; @param  rdi: pointer to request
; @return rax: pointer to request headers
get_request_headers:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+REQ_OFF_HEADERS]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the pointer to the body of the request
; @param  rdi: pointer to request
; @return rax: pointer to request body
get_request_body:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+REQ_OFF_BODY]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

section .bss
response_t:
  .str              resq 1
  .client           resq 1
  .len              resq 1
  .proto            resq 1
  .headers          resq 1
  .body             resq 1
  .content_len      resq 1
  .content_len_str  resq 1
  .status_code      resq 1
end_response_t:

section .data

RESPONSE_STRUCT_LEN equ end_response_t - response_t

RESPONSE_MAX_LEN equ 0x8000

RESPONSE_DEFAULT_LEN equ 0x1000

; offsets
RESPONSE_OFF_STR              equ 0x0
RESPONSE_OFF_CLIENT           equ RESPONSE_OFF_STR + 0x8
RESPONSE_OFF_LEN              equ RESPONSE_OFF_CLIENT + 0x8
RESPONSE_OFF_PROTO            equ RESPONSE_OFF_LEN + 0x8
RESPONSE_OFF_HEADERS          equ RESPONSE_OFF_PROTO + 0x8
RESPONSE_OFF_BODY             equ RESPONSE_OFF_HEADERS + 0x8
RESPONSE_OFF_CONTENT_LEN      equ RESPONSE_OFF_BODY + 0x8
RESPONSE_OFF_CONTENT_LEN_STR  equ RESPONSE_OFF_CONTENT_LEN + 0x8
RESPONSE_OFF_STATUS_CODE      equ RESPONSE_OFF_CONTENT_LEN_STR + 0x8

STR_CONTENT_LENGTH_MAX_LENGTH equ 7 ; add one for null char

EMPTY_CONTENT db "0", NULL_CHAR

; delimiters
space_delim       db " ", NULL_CHAR
CRLF_delim        db CARRIAGE_RETURN, LINE_FEED, NULL_CHAR
DOUBLE_CRLF_delim db CARRIAGE_RETURN, LINE_FEED, CARRIAGE_RETURN, LINE_FEED, NULL_CHAR

section .text
; frees the response structure and all associated resources
; @param  rdi: pointer to the response
; @return rax: return code
response_free:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp]     -> pointer to the response struct

  mov   [rsp], rdi

  test  rdi, rdi
  jz    .error

.free_headers:
  mov   rdi, [rsp]
  call  response_get_headers
  cmp   rax, 0
  jl    .response_free_str

  ; free headers hash table
  mov   rdi, rax
  call  ht_free
  cmp   rax, 0
  jl    .error

.response_free_str:
  mov   rdi, [rsp]
  call  response_get_str
  cmp   rax, 0
  jle   .response_free_body

  mov   rdi, rax
  call  free
  cmp   rax, 0
  jl    .error

.response_free_body:
  mov   rdi, [rsp]
  call  response_get_body
  cmp   rax, 0
  jle   .free_struct

  mov   rdi, rax
  call  free

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

; sets the length of the body of the response
; @param  rdi: pointer to response struct
; @return rax: length of the response content
response_get_content_len:
  test  rdi, rdi
  jz    .error

  mov   rax, [rdi+RESPONSE_OFF_CONTENT_LEN]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the length of the body of the response
; @param  rdi: pointer to response struct
; @param  rsi: content length
; @return rax: return code
response_set_content_len:
  test  rdi, rdi
  jz    .error

  mov   [rdi+RESPONSE_OFF_CONTENT_LEN], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the string representation of the response
; @param  rdi: pointer to response struct
; @param  rsi: pointer to the string
; @return rax: return code
response_set_str:
  test  rdi, rdi
  jz    .error

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
response_get_str:
  test  rdi, rdi
  jz    .error

  mov   rax, [rdi+RESPONSE_OFF_STR]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the response content length string
; @param  rdi: pointer to response
; @return  rax: pointer to string
response_get_content_len_str:
  test  rdi, rdi
  jz    .error

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
response_set_content_len_str:
  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

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
response_get_protocol:
  lea   rax, [HTTP_1_1]
  ret

; returns the response headers
; @param  rdi: pointer to response
; @param  rax: pointer to response headers
response_get_headers:
  test  rdi, rdi
  jz    .error

  mov   rax, [rdi+RESPONSE_OFF_HEADERS]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the response status code
; @param  rdi: pointer to response
; @param  rax: pointer to response status code
response_get_status_code:
  test  rdi, rdi
  jz    .error

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

  ; STACK USAGE
  ; [rsp] -> pointer to the response struct

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
  call  response_set_headers
  cmp   rax, 0
  jl    .error

  ; set body to 0
  mov   rdi, [rsp]
  mov   rsi, NO_ARG
  call  response_set_body
  cmp   rax, 0
  jl    .error

  ; set response content length to 0
  mov   rdi, [rsp]
  mov   rsi, NO_ARG
  call  response_set_content_len
  cmp   rax, 0
  jl    .error

  ; set response str to 0
  mov   rdi, [rsp]
  mov   rsi, NO_ARG
  call  response_set_str
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp]
  jmp   .return

.error:
  mov   rax, [rsp]
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
response_set_headers:
  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

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
response_set_status_code:
  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

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
response_set_body:
  test  rdi, rdi
  jz    .error

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
response_get_body:
  test  rdi, rdi
  jz    .error

  mov   rax, [rdi+RESPONSE_OFF_BODY]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; stringifies the response struct
; @param  rdi: pointer to response struct
; @return rax: pointer to string representation of the response
response_to_str:
  sub   rsp, 0x20
  sub   rsp, HEADERS_MAX_LEN

  ; STACK USAGE
  ; [rsp]       -> pointer to the response struct
  ; [rsp+0x8]   -> pointer to the boeuf holding the stringified response
  ; [rsp+0x10]  -> pointer to the response body
  ; [rsp+0x18]  -> pointer to the response string
  ; [rsp+0x20]  -> pointer to the response headers

  mov   [rsp], rdi
  mov   qword [rsp+0x8], 0
  mov   qword [rsp+0x18], 0

  test  rdi, rdi
  jz    .error

  ; init boeuf
  call  boeuf_init
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, rax
  mov   rsi, RESPONSE_DEFAULT_LEN
  call  boeuf_reserve
  cmp   rax, 0
  jl    .error

  ; get context protocol
  mov   rdi, [rsp]
  call  response_get_protocol
  cmp   rax, 0
  jl    .error

  ; create dynamic buffer
  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, space_delim
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  ; get status code
  mov   rdi, [rsp]
  call  response_get_status_code
  cmp   rax, 0
  jl    .error

  ; get string status code
  mov   rdi, rax
  call  get_status_string
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, CRLF_delim
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  ; get headers
  mov   rdi, [rsp]
  call  response_get_headers
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  lea   rsi, [rsp+0x20]
  call  stringify_headers
  cmp   rax, 0
  jl    .error

  ; append headers
  mov   rdi, [rsp+0x8]
  lea   rsi, [rsp+0x20]
  call  boeuf_append_str
  cmp   rax, 0
  jl    .error

  ; get body
  mov   rdi, [rsp]
  call  response_get_body
  cmp   rax, 0
  jl    .error
  je    .skip_body

  mov   [rsp+0x10], rax

  mov   rdi, [rsp]
  call  response_get_content_len
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x10]
  mov   rdx, rax
  call  boeuf_nappend
  cmp   rax, 0
  jl    .error

.skip_body:
  mov   rdi, [rsp+0x8]
  call  boeuf_to_str
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; save response str
  mov   rdi, [rsp]
  mov   rsi, rax
  call  response_set_str
  cmp   rax, 0
  jl    .error

  ; free boeuf
  mov   rdi, [rsp+0x8]
  call  boeuf_free
  cmp   rax, 0
  jl    .error

  ; return response
  mov   rax, [rsp+0x18]

  jmp   .return

.error:
  ; free response boeuf
  mov   rdi, [rsp+0x8]
  test  rdi, rdi
  jz    .free_boeuf_str

  call  boeuf_free

.free_boeuf_str:
  mov   rdi, [rsp+0x18]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, HEADERS_MAX_LEN
  add   rsp, 0x20
  ret

; sends a response without content
; @param  rdi: pointer to context struct
; @param  rsi: status code
; @return rax: return code
send_no_content:
  sub   rsp, 0x18

  test  rdi, rdi
  jz    .error

  ; default status code is 204 No Content
  mov   rax, NO_CONTENT
  cmp   rsi, 0
  cmove rsi, rax

  ; STACK USAGE
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> status code
  ; [rsp+0x10]  -> pointer to the response

  mov   [rsp], rdi
  mov   qword [rsp+0x8], rsi

  ; validate status code
  mov   rdi, qword [rsp+0x8]
  call  get_status_string
  cmp   rax, 0
  jl    .error

  ; get response
  mov   rdi, [rsp]
  call  ctx_get_response
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; get response headers
  mov   rdi, [rsp+0x10]
  call  response_get_headers
  cmp   rax, 0
  jl    .error

  ; set content length to 0
  mov   rdi, rax
  mov   rsi, CONTENT_LENGTH_HEADER
  lea   rdx, [EMPTY_CONTENT]
  call  set_header
  cmp   rax, 0
  jl    .error

  ; set body to 0
  mov   rdi, [rsp+0x10]
  mov   rsi, 0
  call  response_set_body
  cmp   rax, 0
  jl    .error

  ; send response
  mov   rdi, [rsp]
  mov   rsi, qword [rsp+0x8]
  call  response_send
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  ; return 500
  mov   rdi, [rsp]
  mov   rsi, INTERNAL_SERVER_ERROR
  call  send_no_content

  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; sends a json response
; @param  rdi: pointer to context struct
; @param  rsi: status code
; @param  rdx: pointer to the json
; @return rax: return code
send_json:
  sub   rsp, 0x40

  ; STACK USAGE
  ; [rsp]       -> pointer to context struct
  ; [rsp+0x8]   -> status code
  ; [rsp+0x10]  -> pointer to the json object
  ; [rsp+0x18]  -> pointer to the duplicated json object
  ; [rsp+0x20]  -> pointer to the response struct
  ; [rsp+0x28]  -> pointer to the response headers
  ; [rsp+0x30]  -> content length
  ; [rsp+0x38]  -> content length string

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   qword [rsp+0x18], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  cmp   rdx, 0
  jle   .error

  ; validate status code
  mov   rdi, qword [rsp+0x8]
  call  get_status_string
  cmp   rax, 0
  jl    .error

  ; get response
  mov   rdi, [rsp]
  call  ctx_get_response
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  mov   rdi, [rsp+0x20]
  call  response_get_headers
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x28], rax

  ; set content type headers
  mov   rdi, [rsp+0x28]
  mov   rsi, CONTENT_TYPE_HEADER
  mov   rdx, CONTENT_JSON
  call  set_header
  cmp   rax, 0
  jl    .error

  ; json to string
  mov   rdi, [rsp+0x10]
  call  json_to_str
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; get json length
  mov   rdi, rax
  call  strlen
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x30], rax

  mov   rdi, [rsp+0x20]
  mov   rsi, qword [rsp+0x30]
  call  response_set_content_len
  cmp   rax, 0
  jl    .error

  mov   rdi, qword [rsp+0x30]
  lea   rsi, [rsp+0x38]
  mov   rdx, UNSIGNED_LONG_MAX_STR_SIZE
  call  itoa
  cmp   rax, 0
  jl    .error

  ; set content length
  mov   rdx, rax
  mov   rdi, [rsp+0x28]
  mov   rsi, CONTENT_LENGTH_HEADER
  call  set_header
  cmp   rax, 0
  jl    .error

  ; add json to body
  mov   rdi, [rsp+0x20]
  mov   rsi, [rsp+0x18]
  call  response_set_body
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  call  response_send
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rdi, [rsp+0x18]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  ; return 500
  mov   rdi, [rsp]
  mov   rsi, INTERNAL_SERVER_ERROR
  call  send_no_content

  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x40
  ret

; sends a string response as HTML
; TODO: merge with send string?
; @param  rdi: pointer to context struct
; @param  rsi: status code
; @param  rdx: pointer to the HTML
; @return rax: return code
send_html:
  sub   rsp, 0x40

  ; STACK USAGE
  ; [rsp]       -> pointer to context struct
  ; [rsp+0x8]   -> status code
  ; [rsp+0x10]  -> pointer to the html string
  ; [rsp+0x18]  -> pointer to the duplicated string
  ; [rsp+0x20]  -> pointer to the response struct
  ; [rsp+0x28]  -> pointer to the response headers
  ; [rsp+0x30]  -> content length
  ; [rsp+0x38]  -> content length string

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   qword [rsp+0x18], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  cmp   rdx, 0
  jle   .error

  ; validate status code
  mov   rdi, qword [rsp+0x8]
  call  get_status_string
  cmp   rax, 0
  jl    .error

  ; get response
  mov   rdi, [rsp]
  call  ctx_get_response
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  mov   rdi, [rsp+0x20]
  call  response_get_headers
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x28], rax

  ; set content type headers
  mov   rdi, [rsp+0x28]
  mov   rsi, CONTENT_TYPE_HEADER
  mov   rdx, CONTENT_HTML
  call  set_header
  cmp   rax, 0
  jl    .error

  ; get string length
  mov   rdi, [rsp+0x10]
  call  strlen
  cmp   rax, 0
  jl    .error

  mov   qword [rsp+0x30], rax

  ; duplicate string
  mov   rdi, [rsp+0x10]
  mov   rsi, qword [rsp+0x30]
  call  strndup
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  mov   rdi, [rsp+0x20]
  mov   rsi, qword [rsp+0x30]
  call  response_set_content_len
  cmp   rax, 0
  jl    .error

  mov   rdi, qword [rsp+0x30]
  lea   rsi, [rsp+0x38]
  mov   rdx, UNSIGNED_LONG_MAX_STR_SIZE
  call  itoa
  cmp   rax, 0
  jl    .error

  ; set content length
  mov   rdx, rax
  mov   rdi, [rsp+0x28]
  mov   rsi, CONTENT_LENGTH_HEADER
  call  set_header
  cmp   rax, 0
  jl    .error

  ; add string to body
  mov   rdi, [rsp+0x20]
  mov   rsi, [rsp+0x18]
  call  response_set_body
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  call  response_send
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rdi, [rsp+0x18]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rdi, [rsp]
  mov   rsi, INTERNAL_SERVER_ERROR
  call  send_no_content

  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x40
  ret

; sends a string response
; @param  rdi: pointer to context struct
; @param  rsi: status code
; @param  rdx: pointer to string
; @return rax: return code
send_string:
  sub   rsp, 0x40

  ; STACK USAGE
  ; [rsp]       -> pointer to context struct
  ; [rsp+0x8]   -> status code
  ; [rsp+0x10]  -> pointer to string to send
  ; [rsp+0x18]  -> pointer to the duplicated string
  ; [rsp+0x20]  -> pointer to response struct
  ; [rsp+0x28]  -> pointer to response headers
  ; [rsp+0x30]  -> string length
  ; [rsp+0x38]  -> content length string

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   qword [rsp+0x18], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  cmp   rdx, 0
  jle   .error

  ; validate status code
  mov   rdi, qword [rsp+0x8]
  call  get_status_string
  cmp   rax, 0
  jl    .error

  ; get response
  mov   rdi, [rsp]
  call  ctx_get_response
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  mov   rdi, [rsp+0x20]
  call  response_get_headers
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x28], rax

  ; set content type headers
  mov   rdi, [rsp+0x28]
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

  mov   qword [rsp+0x30], rax

  ; duplicate string
  mov   rdi, [rsp+0x10]
  mov   rsi, qword [rsp+0x30]
  call  strndup
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  mov   rdi, [rsp+0x20]
  mov   rsi, qword [rsp+0x30]
  call  response_set_content_len
  cmp   rax, 0
  jl    .error

  mov   rdi, qword [rsp+0x30]
  lea   rsi, [rsp+0x38]
  mov   rdx, UNSIGNED_LONG_MAX_STR_SIZE
  call  itoa
  cmp   rax, 0
  jl    .error

  ; set content length
  mov   rdx, rax
  mov   rdi, [rsp+0x28]
  lea   rsi, [CONTENT_LENGTH_HEADER]
  call  set_header
  cmp   rax, 0
  jl    .error

  ; add string to body
  mov   rdi, [rsp+0x20]
  mov   rsi, [rsp+0x18]
  call  response_set_body
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  call  response_send
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rdi, [rsp+0x18]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rdi, [rsp]
  mov   rsi, INTERNAL_SERVER_ERROR
  call  send_no_content

  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x40
  ret

; sends response to client fd
; @param  rdi: pointer to context struct
; @param  rsi: status code (int)
; @return rax: return code
response_send:
  sub   rsp, 0x30

  ; STACK USAGE
  ; [rsp]       -> pointer to the context struct
  ; [rsp+0x8]   -> status code (int)
  ; [rsp+0x10]  -> pointer to the context response
  ; [rsp+0x18]  -> pointer to the stringified response
  ; [rsp+0x20]  -> length of the response

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x18], 0

  test  rdi, rdi
  jz    .error

  test  rsi, rsi
  jz    .error

  ; validate status code
  mov   rdi, qword [rsp+0x8]
  call  get_status_string
  cmp   rax, 0
  jl    .error

  ; get response
  mov   rdi, [rsp]
  call  ctx_get_response
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  ; set connection header
  mov   rdi, [rsp+0x10]
  call  response_get_headers
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  lea   rsi, [CONNECTION_HEADER]
  lea   rdx, [CONNECTION_KEEP_ALIVE]
  call  set_header
  cmp   rax, 0
  jl    .error

  ; set response status code
  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp+0x8]
  call  response_set_status_code
  cmp   rax, 0
  jl    .error

  ; stringify response
  mov   rdi, [rsp+0x10]
  call  response_to_str
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; get response length
  mov   rdi, [rsp+0x18]
  call  strlen
  cmp   rax, 0
  jl    .error

  mov   qword [rsp+0x20], rax

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

  ; write response
  mov   rdi, rax
  mov   rax, SYS_WRITE
  mov   rsi, [rsp+0x18]
  mov   rdx, qword [rsp+0x20]
  syscall
  cmp   rax, 0
  jl    .error

  jmp   .return

.error:
  ; make sure you free the response
  mov   rax, qword [rsp+0x18]
  test  rax, rax
  jz    .no_free

  mov   rdi, rax
  call  boeuf_free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  ; ctx is done
  mov   rdi, [rsp]
  mov   qword [rdi+CTX_OFF_DONE], 1

  add   rsp, 0x30
  ret

; TODO: make sure we have permissions
; serves a static file
; @param  rdi: ctx
; @param  rsi: file path
; @return rax: void
send_static_file:
  sub   rsp, 0x50

  ; STACK USAGE
  ; [rsp]       -> pointer to context struct
  ; [rsp+0x8]   -> pointer to file path string
  ; [rsp+0x10]  -> pointer to content type header
  ; [rsp+0x18]  -> pointer to response struct
  ; [rsp+0x20]  -> pointer to response headers
  ; [rsp+0x28]  -> file descriptor
  ; [rsp+0x30]  -> pointer to file content
  ; [rsp+0x38]  -> content length
  ; [rsp+0x40]  -> content length string

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  ; get file ext
  mov   rdi, rsi
  call  get_content_type
  cmp   rax, 0
  jl    .error_open

  mov   [rsp+0x10], rax ; save content type header

  ; get context response
  mov   rdi, [rsp]
  call  ctx_get_response
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; get response headers
  mov   rdi, [rsp+0x18]
  call  response_get_headers
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

  ; get length of the file
  mov   rdi, rax
  call  get_file_length
  cmp   rax, 0
  jl    .error

  ; save length of the file
  mov   qword [rsp+0x38], rax

  mov   rdi, [rsp+0x18]
  mov   rsi, rax
  call  response_set_content_len
  cmp   rax, 0
  jl    .error

  mov   rdi, qword [rsp+0x38]
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x30], rax

  ; read file
  mov   rdi, qword [rsp+0x28]
  mov   rsi, rax
  mov   rdx, qword [rsp+0x38]
  call  read_file
  cmp   rax, 0
  jl    .error_read

  mov   rdi, rax
  lea   rsi, [rsp+0x40]
  mov   rdx, UNSIGNED_LONG_MAX_STR_SIZE
  call  itoa
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
  call  response_set_body
  cmp   rax, 0
  jl    .error

  ; send response
  mov   rdi, [rsp]
  mov   rsi, OK
  call  response_send
  cmp   rax, 0
  jl    .error

  ; close the file
  mov   rdi, qword [rsp+0x28]
  call  close_file

  mov   rax, SUCCESS_CODE

  ; clean stack
  jmp   .return

.error_open:
  ; load 404
  mov   rdi, [rsp]
  mov   rsi, NOT_FOUND
  call  send_no_content

  mov   rdi, [rsp]
  call  ctx_abort

  ; this is not an error
  mov   rax, SUCCESS_CODE

  jmp   .return

.error_read:
  ; load 500
  mov   rdi, [rsp]
  mov   rsi, INTERNAL_SERVER_ERROR
  call  send_no_content

  mov   rdi, [rsp]
  call  ctx_abort

.error:
  ; can fail as we return an error anyway
  mov   rdi, qword [rsp+0x28]
  call  close_file

  mov   rax, FAILURE_CODE

.return:
  ; clean stack
  add   rsp, 0x50
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
  call  ctx_get_request
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
  ; check if the ctx has been aborted
  mov   rdi, [rsp]
  call  ctx_is_aborted
  cmp   rax, TRUE
  je    .is_aborted

  ; send 500
  mov   rdi, [rsp]
  mov   rsi, INTERNAL_SERVER_ERROR
  call  send_no_content

  mov   rdi, [rsp]
  call  ctx_abort

.is_aborted:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; creates a json response with success, message and object
; user is responsible of freeing the json object
; @param  rdi: success (bool)
; @param  rsi: pointer to the message or 0
; @param  rdx: pointer to the object or 0
; @return rax: pointer to the json response object
create_json_response:
  sub   rsp, 0x28

  ; STACK USAGE
  ; [rsp]       -> success
  ; [rsp+0x8]   -> pointer to the message
  ; [rsp+0x10]  -> pointer to the json object
  ; [rsp+0x18]  -> pointer to the stringified json object
  ; [rsp+0x20]  -> pointer to the response object

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   qword [rsp+0x18], 0
  mov   qword [rsp+0x20], 0

  cmp   rdi, FALSE
  jl    .error
  cmp   rdi, TRUE
  jg    .error

  call  json_create
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  mov   rdi, [rsp+0x18]
  mov   rsi, success_key
  mov   rdx, [rsp]
  call  json_insert_bool
  cmp   rax, 0
  jl    .error

  mov   rdx, [rsp+0x8]
  test  rdx, rdx
  jz    .skip_message

  mov   rdi, [rsp+0x18]
  mov   rsi, message_key
  call  json_insert_string
  cmp   rax, 0
  jl    .error

.skip_message:
  mov   rdx, [rsp+0x10]
  test  rdx, rdx
  jz    .skip_data

  ; data to string
  mov   rdi, [rsp+0x10]
  call  json_to_str
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  mov   rdi, [rsp+0x18]
  mov   rsi, data_key
  mov   rdx, rax
  call  json_insert_object
  cmp   rax, 0
  jl    .error

  ; free json to str
  mov   rdi, [rsp+0x20]
  call  free
  cmp   rax, 0
  jl    .error

.skip_data:
  mov   rdi, [rsp+0x18]
  call  json_end
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x18]

  jmp   .return

.error:
  mov   rdi, [rsp+0x18]
  test  rdi, rdi
  jz    .free_json_str

  call  json_free

.free_json_str:
  mov   rdi, [rsp+0x20]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x28
  ret

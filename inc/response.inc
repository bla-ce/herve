section .data
  response_struct:
    res_str         dq 0
    res_client      dq 0    
    res_len         dq 0
    res_proto       dq 0
    res_headers     dq 0
    res_body        dq 0
    res_content_len dq 0
    res_status_code dw 0
  end_response_struct:

  RESPONSE_STRUCT_LEN equ end_response_struct - response_struct

  ; offsets
  RESPONSE_OFF_STR           equ 0x0
  RESPONSE_OFF_CLIENT        equ RESPONSE_OFF_STR + 0x8
  RESPONSE_OFF_LEN           equ RESPONSE_OFF_CLIENT + 0x8
  RESPONSE_OFF_PROTO         equ RESPONSE_OFF_LEN + 0x8
  RESPONSE_OFF_HEADERS       equ RESPONSE_OFF_PROTO + 0x8
  RESPONSE_OFF_BODY          equ RESPONSE_OFF_HEADERS + 0x8
  RESPONSE_OFF_CONTENT_LEN   equ RESPONSE_OFF_BODY + 0x8
  RESPONSE_OFF_STATUS_CODE   equ RESPONSE_OFF_CONTENT_LEN + 0x8

  STR_CONTENT_LENGTH_MAX_LENGTH equ 7 ; add one for null char

  EMPTY_CONTENT db "0", NULL_CHAR

  test_response db "HTTP/1.1 204 No Content", CARRIAGE_RETURN, LINE_FEED 
                db "Content-Length: 0", CARRIAGE_RETURN, LINE_FEED,
                db CARRIAGE_RETURN, LINE_FEED, NULL_CHAR

section .text
; initialise the response struct and malloc necessary resources
; @return rax: pointer to response struct
response_init:
  sub   rsp, 0x8

  ; malloc response
  mov   rdi, RESPONSE_STRUCT_LEN
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   [rsp], rax

  ; malloc headers
  mov   rdi, HEADERS_MAX_LEN
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, rax
  call  set_response_headers

  cmp   rax, 0
  jl    .error

  mov   rax, [rsp]
  jmp   .return

.error:
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

; returns the headers of the response struct
; @param  rdi: pointer to the response struct
; @return rax: pointer to the headers array
get_response_headers:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+RESPONSE_OFF_HEADERS]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; stringifies the response struct
; @param  rdi: pointer to response struct
; @return rax: pointer to string representation of the response
stringify_response:
  lea   rax, [test_response]
  ret

; sends a response without content
; @param  rdi: pointer to context struct
; @return rax: return code
send_no_content:
  sub   rsp, 0x8

  mov   [rsp], rdi

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
  mov   rsi, NO_CONTENT
  call  send_response

  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret


; sends response to client fd
; @param  rdi: pointer to context struct
; @param  rsi: status code (int)
; @return rax: return code
send_response:
  sub   rsp, 0x30

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  ; get response
  call  get_ctx_response
  cmp   rax, 0
  jl    .error
  
  mov   [rsp+0x10], rax

  ; get string status code
  ; mov   rdi, rsi
  ; call  get_status_string

  ; cmp   rax, 0
  ; jl    .error

  ; set response status code
  ; mov   rdi, [rsp+0x10]
  ; mov   rsi, rax
  ; call  set_response_status_code

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
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x30
  ret  

; TODO: make sure we have permissions
; serves a static file
; @param  rdi: file path
; @param  rsi: pointer to request struct
; @return rax: void
serve_static_file:
  sub   rsp, 0x28

  cmp   rdi, 0
  jle   .error

  mov   [rsp], rdi

  ; save request client fd
  mov   [rsp+0x20], rsi

  ; get file ext
  call  get_ext

  cmp   rax, 0
  jl    .error_open

  mov   [rsp+0x18], rax

  ; load file path
  mov   rdi, [rsp]

  ; open file
  mov   rsi, O_RDONLY
  mov   rdx, 0
  call  open_file

  cmp   rax, 0
  jl    .error_open

  mov   qword [rsp+0x10], rax

  ; read file
  mov   rdi, qword [rsp+0x10]
  call  read_file

  cmp   rax, 0
  jl    .error_read

  mov   [rsp+0x8], rax

  ; send response
  lea   rdi, [STR_OK]
  mov   rsi, [rsp+0x18]
  mov   rdx, [rsp+0x8]
  mov   rcx, qword [rsp+0x20]
  call  send_response

  mov   rax, SUCCESS_CODE

  ; free file content
  mov   rdi, [rsp+0x8]
  call  free

  ; clean stack
  add   rsp, 0x28
  ret

.error_open:
  ; load 404

.error_read:
  ; load 500

.error:
  ; clean stack
  add   rsp, 0x28

  ; return
  mov   rax, FAILURE_CODE
  ret

; serve file from directory
; @param  rdi: pointer to request struct
serve_dir:
  mov   rsi, rdi
  mov   rdx, rdi
  add   rdx, REQ_OFF_URL
  mov   rdi, [rdx]
  ; add one to remove '/'
  inc   rdi
  call  serve_static_file

  ret


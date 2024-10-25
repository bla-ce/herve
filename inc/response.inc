%ifndef RESPONSE_INC
%define RESPONSE_INC

send_response:
  ; rdi -> status code str
  ; rsi -> content type
  ; rdx -> body
  sub   rsp, 0x28

  mov   rax, qword [request_client]
  mov   qword [rsp], rax
  mov   [rsp+0x8], rdi
  mov   [rsp+0x10], rsi
  mov   [rsp+0x18], rdx

  ; load protocol
  lea   rdi, [HTTP_1_1]
  call  strlen

  mov   rcx, rax
  lea   rdi, [response]
  lea   rsi, [HTTP_1_1]
  rep   movsb

  ; load space
  xor   rax, rax
  mov   al, SPACE
  stosb

  mov   [rsp+0x20], rdi

  ; load status code
  mov   rdi, [rsp+0x8]
  call  strlen

  mov   rcx, rax
  mov   rdi, [rsp+0x20]
  mov   rsi, [rsp+0x8]
  rep   movsb

  ; load new line
  xor   rax, rax
  mov   al, CARRIAGE_RETURN
  stosb

  mov   al, LINE_FEED
  stosb

  mov   [rsp+0x20], rdi

  ; set connection header
  lea   rdi, [CONNECTION_HEADER]
  lea   rsi, [CONNECTION_CLOSE]
  call  set_header

  ; set content_length
  ; get body
  mov   rdi, [rsp+0x18]
  call  strlen
  test  rax, rax
  jz    .skip_body

  ; set content length header
  mov   rdi, rax
  call  to_string
  lea   rsi, [rax]
  lea   rdi, [CONTENT_LENGTH_HEADER]
  call  set_header

  ; set content type header
  lea   rdi, [CONTENT_TYPE_HEADER]
  mov   rsi, [rsp+0x10]
  call  set_header

.skip_body:
  ; load headers
  mov   rdi, [rsp+0x20]
  call  load_headers

  mov   [rsp+0x20], rax

  ; load body
  mov   rdi, [rsp+0x18]
  call  strlen
  mov   rcx, rax
  lea   rsi, [rdi]
  mov   rdi, [rsp+0x20]
  rep   movsb

  ; get response length
  lea   rdi, [response]
  call  strlen
  mov   rdx, rax

  ; send response
  mov   rax, SYS_WRITE
  mov   rdi, qword [rsp]
  lea   rsi, [response]
  syscall

  ; clean stack
  add   rsp, 0x28

  ret

send_error_response:
  ; rdi -> sockfd
  ; rsi -> status code str
  sub   rsp, 0x18

  mov   qword [rsp], rdi
  mov   [rsp+0x8], rsi

  ; load protocol
  lea   rdi, [HTTP_1_1]
  call  strlen

  mov   rcx, rax
  lea   rdi, [response]
  lea   rsi, [HTTP_1_1]
  rep   movsb

  ; load space
  xor   rax, rax
  mov   al, SPACE
  stosb

  mov   [rsp+0x10], rdi

  ; load status code
  mov   rdi, [rsp+0x8]
  call  strlen

  mov   rcx, rax
  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp+0x8]
  rep   movsb

  ; load new line
  xor   rax, rax
  mov   al, CARRIAGE_RETURN
  stosb

  mov   al, LINE_FEED
  stosb

  mov   [rsp+0x10], rdi

  ; load connection header
  lea   rdi, [CONNECTION_HEADER]
  lea   rsi, [CONNECTION_CLOSE]
  call  set_header

  mov   rdi, [rsp+0x10]
  call  load_headers
  
  lea   rdi, [response]
  call  strlen
  mov   rdx, rax

  ; send response
  mov   rax, SYS_WRITE
  mov   rdi, qword [rsp]
  lea   rsi, [response]
  syscall

  ; clean stack
  add   rsp, 0x18
  ret

serve_string:
  ; rdi -> string
  sub   rsp, 0x8 ; string

  mov   [rsp], rdi
  
  lea   rdi, [STR_OK]
  lea   rsi, [CONTENT_PLAIN]
  mov   rdx, [rsp]
  call  send_response

  add   rsp, 0x8
  ret

serve_static_file:
  ; rdi -> file path
  sub   rsp, 0x20 ; path, length, fd and content-type

  mov   [rsp], rdi

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
  lea   rsi, [file]
  mov   rdx, FILE_MAX_LEN ; TODO: update
  call  read_file

  cmp   rax, 0
  jl    .error_read

  mov   qword [rsp+0x8], rax

  ; send response
  lea   rdi, [STR_OK]
  mov   rsi, [rsp+0x18]
  lea   rdx, [file]
  call  send_response

  ; close file
  mov   rax, SYS_CLOSE
  mov   rdi, qword [rsp+0x10]
  syscall

  ; clear file buffer
  xor   rax, rax
  mov   al, NULL_CHAR
  lea   rdi, [file]
  mov   rcx, FILE_MAX_LEN
  rep   stosb

  mov   rax, 0
  ; clean stack
  add   rsp, 0x20
  ret

.error_open:
  ; load 404
  mov   qword [response_status_code], NOT_FOUND
  mov   rdi, qword [request_client]
  lea   rsi, [STR_NOT_FOUND]
  call  send_error_response
  jmp   .error

.error_read:
  ; load 500
  mov   qword [response_status_code], INTERNAL_SERVER_ERROR
  mov   rdi, qword [request_client]
  lea   rsi, [STR_INTERNAL_SERVER_ERROR]
  call  send_error_response

.error:
  ; clean stack
  add   rsp, 0x20

  ; return
  mov   rax, FAILURE_CODE
  ret

; serve file from directory
; @param rdi: request
serve_dir:
  lea   rdi, [request_url]  
  ; add one to remove '/'
  inc   rdi
  call  serve_static_file

  ret

%endif


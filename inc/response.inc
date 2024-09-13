%ifndef RESPONSE_INC
%define RESPONSE_INC

send_response:
  ; rdi -> sockfd
  ; rsi -> status code str
  ; rdx -> content type
  ; rcx -> body
  sub   rsp, 0x28

  mov   qword [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   [rsp+0x18], rcx

  ; load protocol
  lea   rdi, [request_proto]
  call  get_length

  mov   rcx, rax
  lea   rdi, [response]
  lea   rsi, [request_proto]
  rep   movsb

  ; load space
  xor   rax, rax
  mov   al, SPACE
  stosb

  mov   [rsp+0x20], rdi

  ; load status code
  lea   rax, [rsp+0x8]
  mov   rdi, [rax]
  call  get_length

  mov   rcx, rax
  lea   rax, [rsp+0x20]
  mov   rdi, [rax]
  lea   rax, [rsp+0x8]
  mov   rsi, [rax]
  rep   movsb

  ; load new line
  xor   rax, rax
  mov   al, CARRIAGE_RETURN
  stosb

  mov   al, LINE_FEED
  stosb

  mov   rax, [rsp+0x10]
  cmp   rax, 0
  je    skip_body

  mov   [rsp+0x20], rdi

  ; load content type
  lea   rax, [rsp+0x10]
  mov   rdi, [rax]
  call  get_length
  mov   rcx, rax
  lea   rsi, [rdi]
  lea   rax, [rsp+0x20]
  mov   rdi, [rax]
  rep   movsb

  ; load new line
  xor   rax, rax
  mov   al, CARRIAGE_RETURN
  stosb

  mov   al, LINE_FEED
  stosb

  mov   [rsp+0x20], rdi

  ; load connection
  lea   rdi, [CONNECTION_CLOSE]
  call  get_length
  mov   rcx, rax
  lea   rsi, [rdi]
  lea   rax, [rsp+0x20]
  mov   rdi, [rax]
  rep   movsb

  ; load new lines
  lea   rsi, [post_headers]
  mov   rcx, post_headers_len
  rep   movsb
  
  mov   [rsp+0x20], rdi

  ; load body
  lea   rax, [rsp+0x18]
  mov   rdi, [rax]
  call  get_length
  mov   rcx, rax
  lea   rsi, [rdi]
  lea   rax, [rsp+0x20]
  mov   rdi, [rax]
  rep   movsb

skip_body:
  ; get response length
  lea   rdi, [response]
  call  get_length
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
  lea   rdi, [request_proto]
  call  get_length

  mov   rcx, rax
  lea   rdi, [response]
  lea   rsi, [request_proto]
  rep   movsb

  ; load space
  xor   rax, rax
  mov   al, SPACE
  stosb

  mov   [rsp+0x10], rdi

  ; load status code
  lea   rax, [rsp+0x8]
  mov   rdi, [rax]
  call  get_length

  mov   rcx, rax
  lea   rax, [rsp+0x10]
  mov   rdi, [rax]
  lea   rax, [rsp+0x8]
  mov   rsi, [rax]
  rep   movsb

  ; load new line
  xor   rax, rax
  mov   al, CARRIAGE_RETURN
  stosb

  mov   al, LINE_FEED
  stosb

  ; send response
  mov   rax, SYS_WRITE
  mov   rdi, qword [rsp]
  lea   rsi, [response]
  mov   rdx, RESPONSE_MAX_LEN
  syscall

  ; clean stack
  add   rsp, 0x18
  ret

serve_string:
  ; rdi -> string
  sub   rsp, 0x10 ; string and length

  mov   [rsp], rdi

  call  get_length
  mov   qword [rsp+0x8], rax
  
  lea   rdi, [request_client]
  lea   rsi, [STR_OK]
  lea   rdx, [CONTENT_PLAIN]
  lea   rax, [rsp]
  mov   rcx, [rax]
  call  send_response

  add   rsp, 0x10
  ret

%endif

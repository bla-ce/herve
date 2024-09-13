%ifndef RESPONSE_INC
%define RESPONSE_INC

send_response:
  ; rdi -> sockfd
  ; rsi -> status code str
  ; rdx -> content type
  ; rcx -> body
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

b1:
  ; send response
  mov   rax, SYS_WRITE
  mov   rdi, qword [rsp]
  lea   rsi, [response]
  mov   rdx, RESPONSE_MAX_LEN
  syscall

  ; clean stack
  add   rsp, 0x18
  ret

%endif

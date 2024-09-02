%ifndef REQUEST_INC
%define REQUEST_INC

parse_request:
  lea   rsi, [request]
  mov   rcx, qword [request_len]

  ; extract method
  push  rsi

  lea   rdi, [rsi]
  mov   rsi, SPACE
  call  find_next_char
  cmp   rax, 0x0
  jl    parse_request_error

  lea   rdi, [request_method]
  pop   rsi

  mov   rcx, rax
  rep   movsb

  ; mov rsi to go to the url
  inc   rsi

  ; extract url
  push  rsi

  lea   rdi, [rsi]
  mov   rsi, SPACE
  call  find_next_char
  cmp   rax, 0x0
  jl    parse_request_error

  lea   rdi, [request_url]
  pop   rsi

  mov   rcx, rax
  rep   movsb

  ; inc rsi to go to the protocol 
  inc   rsi

  ; extract proto
  push  rsi

  lea   rdi, [rsi]
  mov   rsi, CARRIAGE_RETURN
  call  find_next_char
  cmp   rax, 0x0
  jl    parse_request_error

  lea   rdi, [request_proto]
  pop   rsi

  mov   rcx, rax
  rep   movsb

  push  rsi

  ; parse proto
  lea   rdi, [request_proto]
  mov   rsi, SLASH
  call  find_next_char
  cmp   rax, 0x0
  jl    parse_request_error

  pop   rsi

  ; mov to proto major
  inc   rsi
  sub   rsi, rax

  xor   rax, rax
  lodsb
  sub   al, 0x30  ; ascii to int

  ; check proto major is digit
  cmp   eax, 0x0
  jl    parse_request_error

  cmp   eax, 0x9
  jg    parse_request_error

  mov   dword [request_proto_major], eax

  ; mov to proto minor
  inc   rsi

  xor   rax, rax
  lodsb
  sub   al, 0x30  ; ascii to int

  ; check proto minor is digit
  cmp   eax, 0x0
  jl    parse_request_error

  cmp   eax, 0x9
  jg    parse_request_error

  mov   dword [request_proto_minor], eax

  ; mov to new line (skip 2 char \r\n)
  add   rsi, 0x2

  ; get headers
  push  rsi

  lea   rdi, [rsi]
  mov   rsi, qword [request_len]
  lea   rdx, [post_headers]
  mov   rcx, post_headers_len
  call  strpos

  pop rsi

  lea   rdi, [request_headers]
  mov   rcx, rax
  rep   movsb

  ; get body
  ; move to new line (skip 4 char \r\n\r\n)
  add   rsi, 0x4

  push  rsi

  lea   rdi, [request_body]
  mov   rsi, NULL_CHAR

  call  find_next_char
  cmp   rax, 0x0
  jl    parse_request_error

  lea   rdi, [request_body]
  pop   rsi

  mov   rcx, rax
  rep   movsb

  jmp   parse_request_return

parse_request_error:
  mov   qword [response_status_code], BAD_REQUEST

parse_request_return:
  ret

verify_request:
  lea   rdi, [request_method]
  call  get_length
  mov   r10, rax

  lea   rdi, [request_method]
  mov   rsi, r10
  call  check_method
  cmp   rax, 0
  jl    verify_request_method_not_allowed

  lea   rdi, [request_url]
  call  get_length
  mov   r10, rax

  lea   rdi, [request_url]
  mov   rsi, r10
  call  check_route
  cmp   rax, 0
  jl    verify_request_not_found

  mov   qword [response_status_code], OK
  jmp   verify_request_return

verify_request_method_not_allowed:
  mov   qword [response_status_code], METHOD_NOT_ALLOWED
  jmp   verify_request_return

verify_request_not_found:
  mov   qword [response_status_code], NOT_FOUND
  jmp   verify_request_return

verify_request_return:
  ret

send_response:
  ; rdi -> client
  cmp   qword [response_status_code], OK 
  je    send_200

  cmp   qword [response_status_code], BAD_REQUEST
  je    send_400

  cmp   qword [response_status_code], NOT_FOUND
  je    send_404

  cmp   qword [response_status_code], METHOD_NOT_ALLOWED
  je    send_405

  jmp   return

send_200:
  mov   rax, SYS_WRITE
  mov   rsi, response_200
  mov   rdx, response_200_len
  syscall

  jmp   return

send_400:
  mov   rax, SYS_WRITE
  mov   rsi, response_400
  mov   rdx, response_400_len
  syscall

  jmp   return

send_404:
  mov   rax, SYS_WRITE
  mov   rsi, response_404
  mov   rdx, response_404_len
  syscall

  jmp   return

send_405:
  mov   rax, SYS_WRITE
  mov   rsi, response_405
  mov   rdx, response_405_len
  syscall

  jmp   return

send_500:
  mov   rax, SYS_WRITE
  mov   rsi, response_500
  mov   rdx, response_500_len
  syscall

  jmp   return

return:
  ret

%endif

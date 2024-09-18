%ifndef REQUEST_INC
%define REQUEST_INC

parse_request:
  sub   rsp, 0x8

  lea   rsi, [request]
  mov   [rsp], rsi

  mov   rcx, qword [request_len]

  ; extract method
  lea   rdi, [rsi]
  mov   rsi, SPACE
  call  find_next_char
  cmp   rax, 0x0
  jl    parse_request_error

  mov   rcx, METHOD_MAX_LEN
  cmp   rax, rcx
  cmovg rax, rcx

  lea   rdi, [request_method]
  mov   rcx, rax
  lea   rax, [rsp]
  mov   rsi, [rax] 
  rep   movsb

  ; mov rsi to go to the url
  inc   rsi

  ; extract url
  mov   [rsp], rsi

  lea   rdi, [rsi]
  mov   rsi, SPACE
  call  find_next_char
  cmp   rax, 0x0
  jl    parse_request_error

  cmp   rax, URL_MAX_LEN
  jg    request_uri_too_long

  lea   rdi, [request_url]
  mov   rcx, URL_MAX_LEN
  cmp   rax, rcx
  cmovg rax, rcx

  mov   rcx, rax
  lea   rax, [rsp]
  mov   rsi, [rax]
  rep   movsb

  ; inc rsi to go to the protocol 
  inc   rsi

  ; extract proto
  mov   [rsp], rsi

  lea   rdi, [rsi]
  mov   rsi, CARRIAGE_RETURN
  call  find_next_char
  cmp   rax, 0x0
  jl    parse_request_error

  mov   rcx, rax
  lea   rdi, [request_proto]
  lea   rax, [rsp]
  mov   rsi, [rax]
  rep   movsb

  mov   [rsp], rsi

  ; parse proto
  lea   rdi, [request_proto]
  mov   rsi, SLASH
  call  find_next_char
  cmp   rax, 0x0
  jl    parse_request_error

  mov   rcx, rax

  lea   rax, [rsp]
  mov   rsi, [rax]

  ; mov to proto major
  inc   rsi
  sub   rsi, rcx

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
  mov   [rsp], rsi

  lea   rdi, [rsi]
  mov   rsi, qword [request_len]
  lea   rdx, [post_headers]
  mov   rcx, post_headers_len
  call  strpos

  mov   rcx, rax
  lea   rax, [rsp]
  mov   rsi, [rax]
  lea   rdi, [request_headers]
  rep   movsb

  ; get body
  ; move to new line (skip 4 char \r\n\r\n)
  add   rsi, 0x4

  mov   [rsp], rsi

  lea   rdi, [rsi]
  mov   rsi, NULL_CHAR

  call  find_next_char
  cmp   rax, 0x0
  jl    parse_request_error

  lea   rdi, [request_body]
  mov   rcx, rax
  lea   rax, [rsp]
  mov   rsi, [rax]

  rep   movsb

  mov   rax, 0

  jmp   parse_request_return

parse_request_error:
  mov   qword [response_status_code], BAD_REQUEST
  mov   rdi, qword [request_client]
  lea   rsi, [STR_BAD_REQUEST]
  call  send_error_response

  mov   rax, -1
  jmp   parse_request_return

request_uri_too_long:
  mov   qword [response_status_code], URI_TOO_LONG
  mov   rdi, qword [request_client]
  lea   rsi, [STR_URI_TOO_LONG]
  call  send_error_response

  mov   rax, -1

parse_request_return:
  add   rsp, 0x8
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
  mov   rax, 0

  jmp   verify_request_return

verify_request_method_not_allowed:
  mov   qword [response_status_code], METHOD_NOT_ALLOWED

  mov   rdi, qword [request_client]
  lea   rsi, [STR_METHOD_NOT_ALLOWED]
  call  send_error_response

  mov   rax, -1

  jmp   verify_request_return

verify_request_not_found:
  mov   qword [response_status_code], NOT_FOUND

  mov   rdi, qword [request_client]
  lea   rsi, [STR_NOT_FOUND]
  call  send_error_response

  mov   rax, -1

  jmp   verify_request_return

verify_request_return:
  ret

get_body:
  ; rdi -> request
  lea   rax, [request_body]
  ret

%endif


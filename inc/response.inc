%ifndef RESPONSE_INC
%define RESPONSE_INC

send_response:
  ; rdi -> client
  cmp   qword [response_status_code], BAD_REQUEST
  je    send_400

  cmp   qword [response_status_code], NOT_FOUND
  je    send_404

  cmp   qword [response_status_code], METHOD_NOT_ALLOWED
  je    send_405

send_200:
  ; rdi -> client
  mov   rax, SYS_WRITE
  lea   rsi, [response_200]
  mov   rdx, response_200_len
  syscall

  ret

send_400:
  mov   rax, SYS_WRITE
  mov   rsi, response_400
  mov   rdx, response_400_len
  syscall

  ret

send_404:
  mov   rax, SYS_WRITE
  mov   rsi, response_404
  mov   rdx, response_404_len
  syscall

  ret

send_405:
  mov   rax, SYS_WRITE
  mov   rsi, response_405
  mov   rdx, response_405_len
  syscall

  ret

send_500:
  mov   rax, SYS_WRITE
  mov   rsi, response_500
  mov   rdx, response_500_len
  syscall

  ret

serve_string:
  ; rdi ->  string
  sub   rsp, 0x10

  mov   [rsp], rdi
  call  get_length
  mov   qword [rsp+0x8], rax

  mov   rdi, qword [request_client]
  call  send_200

  ; write to client
  lea   rax, [rsp]
  mov   rsi, [rax]
  mov   rax, SYS_WRITE
  mov   rdi, qword [request_client]
  mov   rdx, qword [rsp+0x8]
  syscall

  cmp   rax, 0
  jl    send_HTML_error
    
  jmp   send_HTML_return

send_string_error:
  mov   rax, -1

send_string_return:
  add   rsp, 0x8
  ret

serve_HTML:
  ; rdi -> file path 
  sub   rsp, 0x10

  ; open file
  mov   rax, SYS_OPEN
  mov   rsi, O_RDONLY
  mov   rdx, 0
  syscall

  cmp   rax, 0
  jl    send_HTML_error
  mov   qword [rsp], rax

  ; read file 
  mov   rax, SYS_READ
  mov   rdi, qword [rsp]
  lea   rsi, [file] 
  mov   rdx, FILE_MAX_LEN
  syscall

  cmp   rax, 0
  jl    send_HTML_error
  mov   qword [rsp+0x8], rax

  mov   rdi, qword [request_client]
  call  send_200

  ; write to client
  mov   rax, SYS_WRITE
  mov   rdi, qword [request_client]
  lea   rsi, [file]
  mov   rdx, qword [rsp+0x8]
  syscall

  cmp   rax, 0
  jl    send_HTML_error
    
  jmp   send_HTML_return

send_HTML_error:
  mov   rax, -1

send_HTML_return:
  add   rsp, 0x10
  ret

%endif

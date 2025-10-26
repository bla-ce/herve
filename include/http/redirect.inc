section .text

; sends a redirect
; @param  rdi: pointer to the context struct
; @param  rsi: status code
; @param  rdx: redirect url
; @returb rax: return code
redirect:
  sub   rsp, 0x18

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  cmp   rdx, 0
  jle   .error

  mov   [rsp], rdi
  mov   qword [rsp+0x8], rsi
  mov   [rsp+0x10], rdx

  ; make sure status code starts with 3
  cmp   rsi, 300
  jl    .error

  cmp   rsi, 399
  jg    .error

  ; get response
  mov   rdi, [rsp]
  call  get_ctx_response

  cmp   rax, 0
  jl    .error

  ; get response headers
  mov   rdi, rax
  call  get_response_headers

  cmp   rax, 0
  jl    .error

  ; set location header
  mov   rdi, rax
  lea   rsi, [LOCATION_HEADER]
  mov   rdx, [rsp+0x10]
  call  set_header

  cmp   rax, 0
  jl    .error

  ; send response
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  call  send_no_content
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret


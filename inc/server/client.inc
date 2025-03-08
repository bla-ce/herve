section .data
  client_struct:
    client_fd       dq 0  
    client_sockaddr dq 0
  client_struct_end:

  CLIENT_STRUCT_LEN equ client_struct_end - client_struct

  CLIENT_OFF_CLIENT_FD equ 0x0
  CLIENT_OFF_SOCKADDR  equ CLIENT_OFF_CLIENT_FD + 0x8

section .text
; sets the file descriptor of the client
; @param  rdi: pointer to the client
; @param  rsi: file descriptor
; @return rax: return code
set_client_fd:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   qword [rdi+CLIENT_OFF_CLIENT_FD], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the file descriptor of the client
; @param  rdi: pointer to the client
; @return rax: file descriptor
get_client_fd:
  cmp   rdi, 0
  jle   .error

  mov   rax, qword [rdi+CLIENT_OFF_CLIENT_FD]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret


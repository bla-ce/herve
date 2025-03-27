section .data
  ENVIRON_LEN equ 0x8000
  PORT_ENV    db "PORT", NULL_CHAR

  environ_file_path db "/proc/self/environ", NULL_CHAR

  environ     dq 0
  environ_fd  dq 0
section .text
; searches the environment list to find the environment variable name
; @param  rdi: pointer to name
; @return rax: pointer to value or -1 if error
getenv:
  sub   rsp, 0x10

  cmp   rdi, 0
  jle   .error

  mov   [rsp], rdi

  call  strlen
  mov   qword [rsp+0x8], rax

  lea   rdi, [environ_file_path]
  mov   rsi, O_RDONLY
  call  open_file
  
  cmp   rax, 0
  jl    .error

  mov   qword [environ_fd], rax

  ; read file
  mov   rdi, rax
  call  read_file

  cmp   rax, 0
  jl    .error

  mov   [environ], rax

  mov   rdi, [environ]
  mov   rsi, [rsp]
  call  strpos

  cmp   rax, 0
  jl    .error

  ; go to the beginning of the key
  mov   rdi, [environ]
  add   rdi, rax

  ; go to beginning of the value ('=' position + 10
  mov   rsi, EQUAL_SIGN
  call  find_next_char

  cmp   rax, 0
  jl    .error

  add   rdi, rax
  inc   rdi       ; add one to go to value

  mov   [rsp], rdi
  
  ; close file
  mov   rdi, qword [environ_fd]
  call  close_file

  ; free environ
  mov   rdi, [environ]
  call  free

  mov   rax, [rsp]

  jmp   .return

.error:
  mov   rdi, qword [environ]
  test  rdi, rdi
  jz    .no_free

  call  free

  mov   rdi, qword [environ_fd]
  call  close_file

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret 


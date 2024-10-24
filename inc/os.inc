section .bss
  environ resb ENVIRON_LEN
section .data
  ENVIRON_LEN equ 0x8000
  PORT_ENV    db "PORT", NULL_CHAR

  environ_file_path db "/proc/self/environ", NULL_CHAR

  environ_fd  dq 0
section .text
; searches the environment list to find the environment variable name
; @param  rdi: pointer to name
; @return rax: pointer to value or -1 if error
getenv:
  sub   rsp, 0x10

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
  lea   rsi, [environ]
  mov   rdx, ENVIRON_LEN
  call  read_file

  cmp   rax, 0
  jl    .error

  lea   rdi, [environ]
  mov   rsi, rax
  mov   rdx, [rsp]
  mov   rcx, qword [rsp+0x8]
  call  strpos

  cmp   rax, 0
  jl    .error

  ; go to the beginning of the key
  lea   rdi, [environ]
  add   rdi, rax

  ; go to beginning of the value ('=' position + 10
  mov   rsi, EQUAL_SIGN
  .b1:
  call  find_next_char

  cmp   rax, 0
  jl    .error

  add   rdi, rax
  inc   rdi       ; add one to go to value

  mov   [rsp], rdi
  
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ; close file
  mov   rdi, qword [environ_fd]
  call  close_file

  mov   rax, [rsp]

  add   rsp, 0x10
  ret 


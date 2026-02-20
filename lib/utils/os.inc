section .data

  env_path db ".env", NULL_CHAR

section .text
; parses the env file and returns a ht of key=value pairs
; user is responsible for freeing the ht
; @param  rdi: file descriptor of the env file
; @return rax: ht
env_parse:
  sub   rsp, 0x40

  ; [rsp]       -> fd of the env file
  ; [rsp+0x8]   -> file length
  ; [rsp+0x10]  -> buffer to read file
  ; [rsp+0x18]  -> dynamic pointer
  ; [rsp+0x20]  -> number of equal sign
  ; [rsp+0x28]  -> pointer to the ht
  ; [rsp+0x30]  -> pointer to the key
  ; [rsp+0x38]  -> pointer to the value

  mov   [rsp], rdi
  mov   qword [rsp+0x10], 0
  mov   qword [rsp+0x28], 0

  cmp   rdi, 0
  jle   .error

  ; get file length
  mov   rdi, [rsp]
  call  get_file_length
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; malloc buffer to read file
  mov   rdi, [rsp+0x8]
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax
  mov   [rsp+0x18], rax

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x10]
  mov   rdx, [rsp+0x8]
  call  read_file
  cmp   rax, 0
  jl    .error

  ; get the number of env variables (count '=')
  mov   rdi, [rsp+0x10]
  mov   rsi, EQUAL_SIGN
  call  count_char
  cmp   rax, 0
  jl    .error
  je    .null

  mov   [rsp+0x20], rax

  mov   rdi, [rsp+0x20]
  call  ht_create
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x28], rax

.loop:
  ; get buffer and go to next '='
  mov   rdi, [rsp+0x18]
  mov   rsi, EQUAL_SIGN
  call  find_next_char
  cmp   rax, 0
  jl    .loop_end

  mov   rdi, [rsp+0x18]
  mov   [rsp+0x30], rdi
  add   rdi, rax
  mov   rax, NULL_CHAR
  stosb

  mov   [rsp+0x38], rdi

  ; get next line
  mov   rdi, [rsp+0x38]
  mov   rsi, LINE_FEED
  call  find_next_char
  cmp   rax, 0
  jl    .insert   ; last key

  mov   rdi, [rsp+0x38]
  add   rdi, rax
  mov   rax, NULL_CHAR
  stosb

  ; save next pointer
  mov   [rsp+0x18], rdi

.insert:
  mov   rdi, [rsp+0x28]
  mov   rsi, [rsp+0x30]
  mov   rdx, [rsp+0x38]
  call  ht_insert
  cmp   rax, 0
  jl    .error

  jmp   .loop

.loop_end:
  ; free buffer
  mov   rdi, [rsp+0x10]
  call  free
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x28]

  jmp   .return

.null:
  xor   rax, rax
  jmp   .return

.error:
  mov   rdi, [rsp+0x28]
  test  rdi, rdi
  jz    .free_buf

  call  ht_free

.free_buf:
  mov   rdi, [rsp+0x10]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x40
  ret

; read .env file and extracts environment variables
; @param  rdi: path to the env file (.env if null)
; @return rax: pointer to the hash table with env var
env_read:
  sub   rsp, 0x18

  ; STACK USAGE
  ; [rsp]       -> path to the .env file
  ; [rsp+0x8]   -> file descriptor of the .env file
  ; [rsp+0x10]  -> pointer to the ht

  mov   [rsp], rdi
  mov   qword [rsp+0x10], 0

  cmp   rdi, 0
  jg    .path_set

  mov   qword [rsp], env_path

.path_set:
  ; open env file
  mov   rdi, [rsp]
  mov   rsi, O_RDONLY
  mov   rdx, NO_ARG
  call  open_file
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, [rsp+0x8]
  call  env_parse
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  mov   rdi, [rsp+0x8]
  call  close_file
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x10]

  jmp   .return

.error:
  mov   rdi, [rsp+0x8]
  call  close_file

  mov   rdi, [rsp+0x10]
  test  rdi, rdi
  jz    .no_free

  call  ht_free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; terminates the current process
; @param  rdi: return code
exit:
  mov   rax, SYS_EXIT
  syscall

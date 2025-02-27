section .data
  DELIMITER db "%%", NULL_CHAR
  DELIMITER_LENGTH equ $ - DELIMITER - 1 ; minus null char

  tmp_file db "examples/views/", NULL_CHAR

section .text

; parses the template with am hash table
; @param  rdi: pointer to the file path
; @param  rsi: pointer to the hash table
; @return rax: pointer to the template parsed
parse_template:
  sub   rsp, 0x40

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0  ; hash table can be null
  jl    .error

  mov   [rsp], rdi  
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x10], 0
  
  ; make sure the file has a .apl extension
  mov   rdi, [rsp]
  call  get_content_type

  cmp   rax, 0
  jl    .error

  cmp   rax, CONTENT_APL
  jne   .error

  ; open file
  mov   rdi, [rsp]
  mov   rsi, O_RDONLY
  mov   rdx, 0
  call  open_file

  cmp   rax, 0
  jl    .error

  ; read file
  mov   rdi, rax
  call  read_file

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax
  mov   [rsp+0x18], rax ; dynamic pointer

  ; create temporary file
  lea   rdi, [tmp_file]
  mov   rsi, O_RDWR
  or    rsi, O_TMPFILE
  mov   rdx, 0 
  call  open_file

  cmp   rax, 0
  jl    .error

  ; temp file fd
  mov   qword [rsp+0x30], rax

.loop:
  ; look for the delimiter
  mov   rdi, [rsp+0x18]
  mov   rsi, DELIMITER
  call  strpos

  cmp   rax, 0
  jl    .end_loop

  mov   qword [rsp+0x20], rax  ; save pointer position

  ; write to file
  mov   rax, SYS_WRITE 
  mov   rdi, qword [rsp+0x30]
  mov   rsi, [rsp+0x18]
  mov   rdx, qword [rsp+0x20]
  syscall

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x18]
  add   rdi, rax

  ; add delimiter length
  add   rdi, DELIMITER_LENGTH

  mov   [rsp+0x18], rdi

  ; find the second one
  mov   rsi, DELIMITER
  call  strpos

  cmp   rax, 0
  jl    .end_loop               ; TODO: might need to produce warning here

  mov   qword [rsp+0x28], rax

  mov   rdi, [rsp+0x18]
  mov   rsi, qword [rsp+0x28]
  call  strndup

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  ht_get

  ; TODO: return an error
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x38], rax

  ; get length of the value
  mov   rdi, rax
  call  strlen

  cmp   rax, 0
  jl    .error

  ; write to file the value
  mov   rdx, rax
  mov   rax, SYS_WRITE 
  mov   rdi, qword [rsp+0x30]
  mov   rsi, [rsp+0x38]
  syscall

  cmp   rax, 0
  jl    .error

  ; go after the delimiter
  mov   rdi, [rsp+0x18]
  mov   rax, qword [rsp+0x28]
  add   rdi, rax
  add   rdi, DELIMITER_LENGTH

  mov   [rsp+0x18], rdi

  jmp   .loop

.end_loop:
  ; get end of the file
  mov   rdi, [rsp+0x18]
  call  strlen

  cmp   rax, 0
  jl    .error

  ; write the end of the file
  mov   rdx, rax
  mov   rax, SYS_WRITE
  mov   rdi, qword [rsp+0x30]
  mov   rsi, [rsp+0x18]
  syscall

  cmp   rax, 0
  jl    .error

  ; free original file content
  mov   rdi, [rsp+0x10]
  call  free

  ; get file content (closes the file as well)
  mov   rdi, qword [rsp+0x30]
  call  read_file

  cmp   rax, 0
  jl    .error

  jmp   .return

.error:
  ; close the fd -> might return an error but we exit anyway
  mov   rdi, qword [rsp+0x30]
  call  close_file

  mov   rax, qword [rsp+0x10]
  test  rax, rax
  jz    .no_free

  mov   rdi, rax
  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x40
  ret


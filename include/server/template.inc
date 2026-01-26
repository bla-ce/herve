section .data
  DELIMITER db "%%", NULL_CHAR
  DELIMITER_LENGTH equ $ - DELIMITER - 1 ; minus null char

section .text

; parses the template with hash table
; @param  rdi: pointer to the file path
; @param  rsi: pointer to the hash table
; @return rax: pointer to the template parsed
template_parse:
  sub   rsp, 0x50

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0  ; hash table can be null
  jl    .error

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the file path
  ; [rsp+0x8]   -> pointer to the hash table
  ; [rsp+0x10]  -> file descriptor of the file
  ; [rsp+0x18]  -> length of the file
  ; [rsp+0x20]  -> pointer to the content of the file
  ; [rsp+0x28]  -> dynamic pointer of the file
  ; [rsp+0x30]  -> delimiter position
  ; [rsp+0x38]  -> pointer to the boeuf buffer
  ; [rsp+0x40]  -> duplicated string
  ; [rsp+0x48]  -> encoded string

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x20], 0
  mov   qword [rsp+0x40], 0
  mov   qword [rsp+0x48], 0

  ; make sure the file has a .apl extension
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

  mov   qword [rsp+0x10], rax

  ; get the length of the file
  mov   rdi, rax
  call  get_file_length
  cmp   rax, 0
  jl    .error

  mov   qword [rsp+0x18], rax

  mov   rdi, rax
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax
  mov   [rsp+0x28], rax ; dynamic pointer
  mov   qword [rsp+0x38], 0 ; pointer to the boeuf buffer

  ; read file
  mov   rdi, qword [rsp+0x10]
  mov   rsi, rax
  mov   rdx, qword [rsp+0x18]
  call  read_file
  cmp   rax, 0
  jl    .error

  ; null terminate file content
  mov   rdi, [rsp+0x20]
  add   rdi, rax
  mov   rax, NULL_CHAR
  stosb

.loop:
  mov   rdi, [rsp+0x28]
  mov   rsi, DELIMITER
  call  strpos
  cmp   rax, 0
  jl    .end_loop

  mov   qword [rsp+0x30], rax    ; save delimiter position

  cmp   qword [rsp+0x38], 0
  jne   .skip_boeuf_create

  mov   rdi, [rsp+0x28]
  mov   rsi, rax
  call  boeuf_ncreate
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x38], rax

  jmp   .copy_value

.skip_boeuf_create:
  mov   rdi, [rsp+0x38]
  mov   rsi, [rsp+0x28]
  mov   rdx, qword [rsp+0x30]
  call  boeuf_nappend
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x38], rax

.copy_value:
  ; go after start delimiter
  mov   rdi, [rsp+0x28]
  add   rdi, qword [rsp+0x30]
  add   rdi, DELIMITER_LENGTH

  mov   [rsp+0x28], rdi

  ; find the end delimiter
  mov   rsi, DELIMITER
  call  strpos
  cmp   rax, 0
  jl    .error

  ; save delimiter position
  mov   qword [rsp+0x30], rax

  ; duplicate string so we can get a null terminated string
  mov   rdi, [rsp+0x28]
  mov   rsi, rax
  call  strndup
  cmp   rax, 0
  jl    .error

  mov   qword [rsp+0x40], rax

  ; get hash table value
  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  ht_get
  cmp   rax, 0
  jle   .error

  ; encode value
  mov   rdi, rax
  call  escape_html
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x48], rax

  ; append the value
  mov   rdi, [rsp+0x38]
  mov   rsi, [rsp+0x48]
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x38], rax

  ; go after the delimiter
  mov   rdi, [rsp+0x28]
  add   rdi, qword [rsp+0x30]
  add   rdi, DELIMITER_LENGTH

  mov   [rsp+0x28], rdi

  ; free duplicated string
  mov   rdi, [rsp+0x40]
  call  free
  cmp   rax, 0
  jl    .error

  ; free encoded string
  mov   rdi, [rsp+0x48]
  call  free
  cmp   rax, 0
  jl    .error

  jmp   .loop

.end_loop:
  ; append end of the file
  mov   rdi, [rsp+0x38]
  mov   rsi, [rsp+0x28]
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x38], rax

  ; free original file content
  mov   rdi, [rsp+0x20]
  call  free

  ; close the file
  mov   rdi, qword [rsp+0x10]
  call  close_file

  mov   rax, [rsp+0x38]

  jmp   .return

.error:
  mov   rdi, [rsp+0x20]
  test  rdi, rdi
  jz    .free_dup_string

  call  free

.free_dup_string:
  mov   rdi, [rsp+0x40]
  test  rdi, rdi
  jz    .free_encoded

  call  free

.free_encoded:
  mov   rdi, [rsp+0x48]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x50
  ret

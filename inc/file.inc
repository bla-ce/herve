section .bss

section .data
  ERR_EXT_NOT_SUPP  db "file extension not supported", NULL_CHAR
  ERR_NO_EXT        db "file doesn't have an extension", NULL_CHAR

  stat:
    st_dev      dq 0
    st_ino      dq 0
    st_mode     dd 0
    st_nlink    dq 0
    st_uid      dd 0
    st_gid      dd 0
    st_rdev     dd 0
    __pad       dq 0
    st_size     dq 0
    st_blksize  dq 0
    __pad2      dd 0
    st_blocks   dq 0
    st_atime    dq 0
    st_mtime    dq 0
    st_ctime    dq 0
  endstat:

section .text

; opens a file and return the file descriptor
; @param  rdi: pointer to file_path
; @param  rsi: flags
; @return rax: file descriptor or -1 if error
open_file:
  mov   rax, SYS_OPEN
  ; rdi and rsi already populated
  xor   rdx, rdx
  syscall

  ret

; reads count bytes from file descriptor fd into rsi
; @param  rdi: fd
; @param  rsi: pointer to buf
; @return rax: number of bytes read
read_file:
  sub   rsp, 0x8

  mov   [rsp], rsi

  mov   rax, SYS_FSTAT
  lea   rsi, [st_dev]
  syscall

  mov   rdx, rax

  mov   rax, SYS_READ
  mov   rsi, [rsp]
  ; rest of the registers already populated
  syscall

  add   rsp, 0x8
  ret

; closes file description
; @param  rdi: file description
; @return rax: code
close_file:
  mov   rax, SYS_CLOSE
  syscall

  ret

; extract extension of a file
; @param  rdi: file_path
; @return rax: content type
get_ext:
  sub   rsp, 0x8

  ; find next dot
  mov   rsi, DOT 
  call  find_next_char

  cmp   rax, 0
  jl    .no_ext

  ; go to dot
  add   rdi, rax

  ; make sure there is no extra dot
  mov   rax, DOT
  call  find_next_char

  cmp   rax, 0
  jg    .not_supported

  ; save . position
  mov   [rsp], rdi

  lea   rdi, [HTML_EXT]
  mov   rsi, [rsp]
  call  cmpstr

  cmp   rax, 1
  jne   .check_js

  lea   rax, [CONTENT_HTML]
  jmp   .return

.check_js:
  lea   rdi, [JS_EXT]
  mov   rsi, [rsp]
  call  cmpstr

  cmp   rax, 1
  jne   .check_css

  lea   rax, [CONTENT_JS]
  jmp   .return
  
.check_css:
  lea   rdi, [CSS_EXT]
  mov   rsi, [rsp]
  call  cmpstr

  cmp   rax, 1
  jne   .check_plain

  lea   rax, [CONTENT_CSS]
  jmp   .return
 
.check_plain:
  lea   rdi, [TEXT_EXT]
  mov   rsi, [rsp]
  call  cmpstr

  cmp   rax, 1
  jne   .not_supported

  lea   rax, [CONTENT_PLAIN]
  jmp   .return

.no_ext:
  mov   rax, FAILURE_CODE
  jmp   .return

.not_supported:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret


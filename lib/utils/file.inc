section .data
  ; file constants
  O_RDONLY        equ 0
  O_WRONLY        equ 1
  O_RDWR          equ 2
  O_CREAT         equ 0x40
  O_APPEND        equ 0x400
  O_TMPFILE       equ 0x410000
  O_DIRECTORY     equ 65536
  FILE_PATH_LEN   equ 0x100
  FILE_MAX_LEN    equ 0x2000
  DIR_ENTRIES_LEN equ 0x800
  DT_DIR          equ 0x4
  DT_REG          equ 0x8

  ; file permissions
  S_IRWXU equ 0x1C0
  S_IRUSR equ 0x100
  S_IWUSR equ 0x080
  S_IXUSR equ 0x040
  S_IRWXG equ 0x038
  S_IRGRP equ 0x020
  S_IWGRP equ 0x010
  S_IXGRP equ 0x008
  S_IRWXO equ 0x007
  S_IROTH equ 0x004
  S_IWOTH equ 0x002
  S_IXOTH equ 0x001

  ERR_EXT_NOT_SUPP  db "file extension not supported", NULL_CHAR
  ERR_NO_EXT        db "file doesn't have an extension", NULL_CHAR

  ; file extension
  TEXT_EXT  db ".txt", NULL_CHAR
  CSS_EXT   db ".css", NULL_CHAR
  HTML_EXT  db ".html", NULL_CHAR
  JS_EXT    db ".js", NULL_CHAR
  BIN_EXT   db ".bin", NULL_CHAR
  CSV_EXT   db ".csv", NULL_CHAR
  GIF_EXT   db ".gif", NULL_CHAR
  JPEG_EXT  db ".jpeg", NULL_CHAR
  JPG_EXT   db ".jpg", NULL_CHAR
  JSON_EXT  db ".json", NULL_CHAR
  MP4_EXT   db ".mp4", NULL_CHAR
  PNG_EXT   db ".png", NULL_CHAR
  SVG_EXT   db ".svg", NULL_CHAR
  XML_EXT   db ".xml", NULL_CHAR
  APL_EXT   db ".apl", NULL_CHAR ; template extension

  SEEK_SET equ 0x0
  SEEK_END equ 0x2

  DIRENT_OFF_DRECLEN    equ 0x10
  DIRENT_OFF_FILE_NAME  equ 0x12

section .text
; writes to a file
; @param  rdi: file descriptor
; @param  rsi: pointer to the string
; @param  rdx: count
; @return rax: return value
write_file:
  cmp   rdi, 0
  jl    .error

  cmp   rsi, 0
  jle   .error

  mov   rax, SYS_WRITE
  ; rdi, rsi and rdx already populated
  syscall

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; writes a null terminated cstring to a file
; @param  rdi: file descriptor
; @param  rsi: pointer to the string
; @return rax: return value
write_cstring_file:
  sub   rsp, 0x10

  ; *** STACK USAGE *** ;
  ; [rsp]     -> file descriptor
  ; [rsp+0x8] -> pointer to the string

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  cmp   rdi, 0
  jl    .error

  cmp   rsi, 0
  jle   .error

  mov   rsi, rdi
  call  strlen
  cmp   rax, 0
  jl    .error

  mov   rdx, rax
  mov   rax, SYS_WRITE
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  syscall

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; opens a file and return the file descriptor
; @param  rdi: pointer to file_path
; @param  rsi: flags
; @param  rdx: mode
; @return rax: file descriptor or -1 if error
open_file:
  mov   rax, SYS_OPEN
  ; rdi, rsi and rdx already populated
  syscall

  ret

; copies data between infd to outfd
; @param  rdi: outfd
; @param  rsi: infd
; @param  rdx: count
; @return rax: number of bytes read
send_file:
  mov   r9, rdx
  xor   rdx, rdx
  ; rest of the registers are already populated
  syscall

  ret

; reads count bytes from file descriptor fd
; @param  rdi: fd
; @param  rsi: pointer to the buffer
; @param  rdx: count
; @return rax: bytes read
read_file:
  cmp   rdi, 0
  jl    .error

  cmp   rsi, 0
  jl    .error

  cmp   rdx, 0
  jl    .error

  mov   rax, SYS_READ
  syscall

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the length of a file
; @param  rdi: file descriptor
; @return rax: length of the file
get_file_length:
  sub   rsp, 0x10

  ; *** STACK USAGE *** ;
  ; [rsp]     -> file descriptor
  ; [rsp+0x8] -> size of the file

  mov   [rsp], rdi

  cmp   rdi, 0
  jl    .error

  mov   rax, SYS_LSEEK
  mov   rdi, [rsp]
  mov   rsi, 0
  mov   rdx, SEEK_SET
  syscall

  mov   rax, SYS_LSEEK
  mov   rdi, [rsp]
  mov   rsi, 0
  mov   rdx, SEEK_END
  syscall

  mov   [rsp+0x8], rax

  ; put cursor back to the beginning of the file
  mov   rax, SYS_LSEEK
  mov   rdi, [rsp]
  mov   rsi, 0
  mov   rdx, SEEK_SET
  syscall

  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x8]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; closes file description
; @param  rdi: fd
; @return rax: code
close_file:
  mov   rax, SYS_CLOSE
  syscall

  ret

; receives a message from a file descriptor
; @param  rdi: file descriptor
; @param  rsi: buffer
; @param  rdx: buffer max length
; @return rax: number of bytes received
receive_from_file:
  mov   rax, SYS_RECVFROM
  ; rdi, rsi and rdx already populated
  xor   r10, r10
  xor   r9, r9
  xor   r8, r8
  syscall

  ret

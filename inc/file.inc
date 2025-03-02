section .data
  ; file constants
  O_RDONLY        equ 0
  O_WRONLY        equ 1
  O_RDWR          equ 2
  O_CREAT         equ 0x40
  O_APPEND        equ 0x400
  O_TMPFILE       equ 0x410000
  O_DIRECTORY     equ 65536
  FILE_PATH_LEN   equ 0x40
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

; reads count bytes from file descriptor fd into rsi
; @param  rdi: fd
; @return rax: pointer to content
read_file:
  sub   rsp, 0x18

  mov   qword [rsp], rdi

  ; get file size
  mov   rax, SYS_LSEEK
  mov   rdi, qword [rsp]
  mov   rsi, 0
  mov   rdx, SEEK_END
  syscall

  cmp   rax, 0
  jl    .error
  jg    .file_not_empty

  ; file is empty, close it
  mov   rdi, qword [rsp]
  mov   rax, SYS_CLOSE
  syscall

  jmp   .error

.file_not_empty:
  mov   qword [rsp+0x10], rax

  mov   rdi, rax
  inc   rdi       ; add for null char
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; move cursor back to the beginning of the file
  mov   rax, SYS_LSEEK
  mov   rdi, qword [rsp]
  mov   rsi, 0
  mov   rdx, SEEK_SET
  syscall

  cmp   rax, 0
  jl    .error

  ; read file
  mov   rax, SYS_READ
  mov   rdi, qword [rsp]
  mov   rsi, [rsp+0x8]
  mov   rdx, qword [rsp+0x10]
  syscall

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  add   rdi, qword [rsp+0x10]
  mov   rax, NULL_CHAR
  stosb

  ; close file
  mov   rdi, qword [rsp]
  call  close_file

  mov   rax, [rsp+0x8]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; closes file description
; @param  rdi: fd
; @return rax: code
close_file:
  mov   rax, SYS_CLOSE
  syscall

  ret

; extracts extension of a file and returns content type header
; @param  rdi: pointer to file path
; @return rax: pointer to content type header
get_content_type:
  sub   rsp, 0x8

  cmp   rdi, 0
  jle   .not_supported

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
  jne   .check_apl

  lea   rax, [CONTENT_HTML]
  jmp   .return

.check_apl:
  lea   rdi, [APL_EXT]
  mov   rsi, [rsp]
  call  cmpstr

  cmp   rax, 1
  jne   .check_js

  lea   rax, [CONTENT_APL] ; template extension
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
  jne   .check_bin

  lea   rax, [CONTENT_CSS]
  jmp   .return

.check_bin:
  lea   rdi, [BIN_EXT]
  mov   rsi, [rsp]
  call  cmpstr

  cmp   rax, 1
  jne   .check_csv

  lea   rax, [CONTENT_BIN]
  jmp   .return

.check_csv:
  lea   rdi, [CSV_EXT]
  mov   rsi, [rsp]
  call  cmpstr

  cmp   rax, 1
  jne   .check_gif

  lea   rax, [CONTENT_CSV]
  jmp   .return

.check_gif:
  lea   rdi, [GIF_EXT]
  mov   rsi, [rsp]
  call  cmpstr

  cmp   rax, 1
  jne   .check_jpeg

  lea   rax, [CONTENT_GIF]
  jmp   .return

.check_jpeg:
  lea   rdi, [JPEG_EXT]
  mov   rsi, [rsp]
  call  cmpstr

  cmp   rax, 1
  jne   .check_jpg

  lea   rax, [CONTENT_JPEG]
  jmp   .return

.check_jpg:
  lea   rdi, [JPG_EXT]
  mov   rsi, [rsp]
  call  cmpstr

  cmp   rax, 1
  jne   .check_json

  lea   rax, [CONTENT_JPEG]
  jmp   .return
 
.check_json:
  lea   rdi, [JSON_EXT]
  mov   rsi, [rsp]
  call  cmpstr

  cmp   rax, 1
  jne   .check_mp4

  lea   rax, [CONTENT_JSON]
  jmp   .return
 
.check_mp4:
  lea   rdi, [MP4_EXT]
  mov   rsi, [rsp]
  call  cmpstr

  cmp   rax, 1
  jne   .check_png

  lea   rax, [CONTENT_MP4]
  jmp   .return
 
.check_png:
  lea   rdi, [PNG_EXT]
  mov   rsi, [rsp]
  call  cmpstr

  cmp   rax, 1
  jne   .check_svg

  lea   rax, [CONTENT_PNG]
  jmp   .return
 
.check_svg:
  lea   rdi, [SVG_EXT]
  mov   rsi, [rsp]
  call  cmpstr

  cmp   rax, 1
  jne   .check_xml

  lea   rax, [CONTENT_SVG]
  jmp   .return
 
.check_xml:
  lea   rdi, [XML_EXT]
  mov   rsi, [rsp]
  call  cmpstr

  cmp   rax, 1
  jne   .check_plain

  lea   rax, [CONTENT_XML]
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


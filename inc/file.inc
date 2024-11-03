section .bss

section .data
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

section .text
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

; reads count bytes from file descriptor fd into rsi
; @param  rdi: fd
; @param  rsi: pointer to buf
; @param  rdx: max bytes to read
; @return rax: number of bytes read
read_file:
  sub   rsp, 0x8

  mov   rax, SYS_READ
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


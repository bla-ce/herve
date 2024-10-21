section .bss
section .data
section .text

; print error message on standard output
; @param  rdi: error message
perror:
  mov   rsi, 0 ; we don't know the length
  call  print

  ret


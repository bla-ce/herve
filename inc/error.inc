section .bss
section .data
section .text

; print error message on standard output
; @param  rdi: error message
perror:
  cmp   rdi, 0
  jle   .error

  mov   rsi, 0 ; we don't know the length
  call  print

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret


section .text

; terminates the current process
; @param  rdi: return code
exit:
  mov   rax, SYS_EXIT
  syscall


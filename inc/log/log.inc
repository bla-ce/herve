section .data

; log levels prefixes
fatal_info_level  db "[FATAL] ", NULL_CHAR
error_info_level  db "[ERROR] ", NULL_CHAR
warn_info_level   db "[WARN] ", NULL_CHAR
log_info_level    db "[LOG] ", NULL_CHAR
debug_info_level  db "[DEBUG] ", NULL_CHAR
trace_info_level  db "[TRACE] ", NULL_CHAR

section .text

; prints the error message and exits the programme with -1
; @param  rdi: pointer to the string
; @return rax: return code
fatal:
  push  rbp
  mov   rbp, rsp

  push  rdi

  mov   rdi, fatal_info_level
  call  print
  cmp   rax, 0
  jl    .exit
  
  mov   rdi, [rbp-0x8]
  call  print
  
.exit:
  mov   rdi, FAILURE_CODE
  call  exit

  mov   rsp, rbp
  pop   rbp
  ret

; prints the error message with a new line and exits the 
; programme with -1
; @param  rdi: pointer to the string
; @return rax: return code
fatalln:
  push  rbp
  mov   rbp, rsp

  push  rdi

  mov   rdi, fatal_info_level
  call  print
  cmp   rax, 0
  jl    .exit
  
  mov   rdi, [rbp-0x8]
  call  println
  
.exit:
  mov   rdi, FAILURE_CODE
  call  exit

  mov   rsp, rbp
  pop   rbp
  ret


section .data

HTML_NO_ERR        equ 0
HTML_INVALID_ARG   equ 1
HTML_INVALID_CHAR  equ 2

html_invalid_arg_str         db "invalid argument", NULL_CHAR
html_invalid_char_str        db "invalid character", NULL_CHAR
html_error_code_unknown_str  db "unknown error code", NULL_CHAR

html_errno dq 0

html_log_error db "[HTML ERROR] ", NULL_CHAR

section .text

; converts error code into a string
; @param  rdi: error code
; @return rax: pointer to the string
html_error_code_to_string:
  cmp   rdi, 0
  jl    .error

  cmp   rdi, HTML_INVALID_ARG
  jne   .invalid_char

  mov		rax, html_invalid_arg_str

.invalid_char:
  cmp   rdi, HTML_INVALID_CHAR
  jne   .error

  mov		rax, html_invalid_char_str

  jmp   .return

.error:
  mov		rax, html_error_code_unknown_str

.return:
  ret

; prints the error message from the error code
; @return rax: return code
html_perror:
  mov   rdi, html_log_error
  call  print

  mov   rdi, qword [html_errno]
  call  html_error_code_to_string
  ; ^ this function returns a string even on error

  mov   rdi, rax
  call  println

  ret

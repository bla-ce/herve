section .data

BASE64_NO_ERR        equ 0
BASE64_INVALID_ARG   equ 1
BASE64_INVALID_CHAR  equ 2
BASE64_DECODING_ERR  equ 3
BASE64_ENCODING_ERR  equ 4

base64_invalid_arg_str         db "invalid argument", NULL_CHAR
base64_error_code_unknown_str  db "unknown error code", NULL_CHAR

base64_errno dq 0

base64_log_error db "[BASE64 ERROR] ", NULL_CHAR

section .text

; converts error code into a string
; @param  rdi: error code
; @return rax: pointer to the string
base64_error_code_to_string:
  cmp   rdi, 0
  jl    .error

  cmp   rdi, BASE64_INVALID_ARG
  jne   .error

  mov		rax, base64_invalid_arg_str

  jmp   .return

.error:
  mov		rax, base64_error_code_unknown_str

.return:
  ret

; prints the error message from the error code
; @return rax: return code
base64_perror:
  mov   rdi, base64_log_error
  call  print

  mov   rdi, qword [base64_errno]
  call  base64_error_code_to_string
  ; ^ this function returns a string even on error

  mov   rdi, rax
  call  println

  ret

section .data

BOEUF_NO_ERR        equ 0
BOEUF_INVALID_ARG   equ 1
BOEUF_CORRUPTED_BUF equ 2
BOEUF_BUF_TOO_LARGE equ 3

boeuf_invalid_arg_str         db "invalid argument", NULL_CHAR
boeuf_corrupted_buf_str       db "corrupted buffer", NULL_CHAR
boeuf_buf_too_large_str       db "buffer is too large", NULL_CHAR
boeuf_error_code_unknown_str  db "unknown error code", NULL_CHAR

boeuf_errno dq 0

boeuf_log_error db "[BOEUF ERROR] ", NULL_CHAR

section .text

; converts error code into a string
; @param  rdi: error code
; @return rax: pointer to the string
boeuf_error_code_to_string:
  cmp   rdi, 0
  jl    .error

  cmp   rdi, BOEUF_INVALID_ARG
  jne   .check_corrupted_buf

  mov		rax, boeuf_invalid_arg_str

  jmp   .return

.check_corrupted_buf:
  cmp   rdi, BOEUF_CORRUPTED_BUF
  jne   .check_buf_too_large

  mov		rax, boeuf_corrupted_buf_str

  jmp   .return

.check_buf_too_large:
  cmp   rdi, BOEUF_BUF_TOO_LARGE
  jne   .error

  mov		rax, boeuf_buf_too_large_str

  jmp   .return

.error:
  mov		rax, boeuf_error_code_unknown_str

.return:
  ret

; prints the error message from the error code
; @return rax: return code
boeuf_perror:
  mov   rdi, boeuf_log_error
  call  print

  mov   rdi, qword [boeuf_errno]
  call  boeuf_error_code_to_string
  ; ^ this function returns a string even on error

  mov   rdi, rax
  call  println

  ret

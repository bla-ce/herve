section .data

CONFIG_NO_ERR        equ 0
CONFIG_INVALID_ARG   equ 1
CONFIG_MEMORY_ERROR  equ 2

config_invalid_arg_str         db "invalid argument", NULL_CHAR
config_memory_error_str        db "memory error", NULL_CHAR
config_error_code_unknown_str  db "unknown error code", NULL_CHAR

config_errno dq 0

config_log_error db "[CONFIG ERROR] ", NULL_CHAR

section .text

; converts error code into a string
; @param  rdi: error code
; @return rax: pointer to the string
config_error_code_to_string:
  cmp   rdi, 0
  jl    .error

  cmp   rdi, CONFIG_INVALID_ARG
  jne   .check_memory_error

  mov		rax, config_invalid_arg_str

  jmp   .return

.check_memory_error:
  cmp   rdi, CONFIG_MEMORY_ERROR
  jne   .error

  mov		rax, config_memory_error_str

  jmp   .return

.error:
  mov		rax, config_error_code_unknown_str

.return:
  ret

; prints the error message from the error code
; @return rax: return code
config_perror:
  mov   rdi, config_log_error
  call  print

  mov   rdi, qword [config_errno]
  call  config_error_code_to_string
  ; ^ this function returns a string even on error

  mov   rdi, rax
  call  println

  ret

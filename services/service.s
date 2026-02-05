%include "echo_service.s"

section .data

service_types:
  .echo   equ 1

section .text

; returns the type value from the string representation of the type
; @param  rdi: pointer to the string
; @return rax: service type valuie
map_type_str_to_value:
  ret

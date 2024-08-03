global _start

STDOUT    equ 2

SYS_WRITE equ 1
SYS_EXIT  equ 60

LINE_FEED equ 0xA

SUCCESS_CODE equ 0

_start:
  mov rax, SYS_WRITE
  mov rdi, STDOUT
  mov rsi, hello_world
  mov rdx, len
  syscall

  mov rax, SYS_EXIT
  mov rdi, SUCCESS_CODE
  syscall

section .data:
  hello_world db "Hello, World!", LINE_FEED
  len         equ $ - hello_world

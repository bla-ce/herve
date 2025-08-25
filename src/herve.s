global _start

%include "herve.inc"
%include "os.inc"

section .data
  MIN_CLI_ARGS  equ 2
  INIT_ARGV     equ 4 

  ; commands
  INIT_CMD db "init", NULL_CHAR

  ; log messages
  init_msg db "[LOG] Initialising Herve...", NULL_CHAR

  ; error messages
  invalid_cmd_err db "[ERROR] The command you entered is invalid", LINE_FEED
                  db "Available commands:", LINE_FEED
                  db SPACE, SPACE, "- herve init", NULL_CHAR


  missing_args_err  db "[ERROR] Not enough arguments.", LINE_FEED
                    db "Available commands:", LINE_FEED
                    db SPACE, SPACE, "- herve init", NULL_CHAR

section .text
_start:
  sub   rsp, 0x8

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the cli args
  ; [rsp+0x8]   -> argv
  ; [rsp+0x10]  -> pointer to argc

  ; read cli args
  mov   rdi, [rsp+0x8]    ; offset 0x8 because we sub above
  lea   rsi, [rsp+0x10]
  call  parse_cli
  cmp   rax, 0
  jl    .error

  mov   [rsp], rax

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  call  run_command
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  call  free
  cmp   rax, 0
  jl    .error

  add   rsp, 0x8

  mov   rdi, SUCCESS_CODE
  call  exit

.error:
  add   rsp, 0x8

  mov   rdi, FAILURE_CODE
  call  exit

; parses the cli arguments and run the corresponding command
; @param  rdi: pointer to the cli args
; @param  rsi: argc
; @return rax: return code
run_command:
  sub   rsp, 0x10

  ; *** STACK USAGE *** ;
  ; [rsp]     -> pointer to the cli args
  ; [rsp+0x8] -> argc

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; parse the command
  call  parse_command
  cmp   rax, 0
  jle   .error

  ; run the command
  call  rax
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; parses the cli arguments and return the function to run
; @param  rdi: pointer to the cli args
; @param  rsi: argc
; @return rax: function to call
parse_command:
  sub   rsp, 0x10

  ; *** STACK USAGE *** ;
  ; [rsp]     -> pointer to the cli args
  ; [rsp+0x8] -> argc

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  cmp   rsi, MIN_CLI_ARGS
  jge   .compare

  mov   rdi, missing_args_err
  call  println

  jmp   .error

.compare:
  ; get second argument
  mov   rsi, [rsp]
  mov   rdi, [rsi+0x8]
  mov   rsi, INIT_CMD
  call  strcmp
  cmp   rax, 0
  jl    .error

  cmp   rax, TRUE
  je    .init_cmd

  mov   rdi, invalid_cmd_err
  call  println

  jmp   .error

.init_cmd:
  ; check arguments

  mov   rax, herve_init

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

herve_init:
  mov   rdi, init_msg
  call  println

  ret

section .data
section .text

; parses cli arguments into an array of string pointers
; devs are responsible of freeing the array
; @param  rdi: argc
; @param  rsi: pointer to argv[0]
; @return rax: pointer to array
parse_cli:
  sub   rsp, 0x18

  ; *** STACK USAGE *** ;
  ; [rsp]       -> argc
  ; [rsp+0x8]   -> pointer to argv[0]
  ; [rsp+0x10]  -> array of pointers

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  cmp   rdi, 0
  jl    .error
  je    .null

  cmp   rsi, 0
  jle   .error

  ; malloc array of pointers
  mov   rdi, [rsp]
  mov   rsi, 8
  call  calloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  mov   rdi, rax
  mov   rsi, [rsp+0x8]
  xor   r9, r9

.loop:
  cmp   r9, [rsp]
  jge   .end_loop

  mov   rax, [rsi]
  mov   [rdi+r9*8], rax

  inc   r9
  add   rsi, 8

  jmp   .loop

.end_loop:
  mov   rax, [rsp+0x10]

  jmp   .return

.null:
  xor   rax, rax
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret


%ifndef STRING_INC
%define STRING_INC

; compare rdi and rsi
; @param  rdi: str
; @param  rsi: substr
; @return rax: rdi == rsi
cmpstr:
  push  rbp
  mov   rbp, rsp

  push  rdi
  push  rsi

  call  strlen
  test  rax, rax
  jz    .str_not_equal
  mov   rcx, rax

  pop   rsi
  pop   rdi
  rep   cmpsb
  jne   .str_not_equal

  mov   rax, 1
  jmp   .return

.str_not_equal:
  mov   rax, 0

.return:
  leave
  ret

; copy the string pointed to by src, into a string at the buffer pointed to by dst
; @param  rdi: dst
; @param  rsi: src
strcpy:
  push  rbp
  mov   rbp, rsp

  push  rdi
  push  rsi

  lea   rdi, [rsi]
  call  strlen
  mov   rcx, rax

  pop   rsi
  pop   rdi

  rep   movsb

  mov   rax, SUCCESS_CODE

  leave
  ret

; converts int in rdi into string
; @param  rdi: int
; @return rax: pointer to str(int)
to_string:
  cmp   rdi, 1000000
  jge   .invalid_number

  push  rdi
  
  ; load buffer
  lea   rdi, [to_string_buf]

  push  rdi

  ; clear buffer
  mov   al, NULL_CHAR
  mov   rcx, TO_STRING_MAX
  rep   stosb

  pop   rdi

  ; go to the end of the buf (print backward)
  add   rdi, TO_STRING_MAX

  ; go back two to let an extra null char at the end
  sub   rdi, 2
  
  pop   rax
  
.loop:
  xor   rdx, rdx
  mov   rcx, 0xA              ; divisor, dividend in rax
  div   rcx                   ; quotient in rax and remainder in edx

  add   rdx, ZERO_CHAR

  mov   byte [rdi], dl

  dec   rdi

  cmp   rax, 0
  jne   .loop

  inc   rdi
 
  lea   rax, [rdi]

  jmp   .return

.invalid_number:
  mov   rax, -1

.return:
  ret

; returns the index of the next occurence of the char
; relative to current address
; returns the preserved address into rdi
find_next_char:
  ; rdi -> string
  ; rsi -> char
  ; return value -> rax: index
  push  rsi

  lea   rsi, [rdi]

  pop   rdx

  sub   rsp, 0x8
  mov   qword [rsp], 0

.loop:
  mov   al, byte [rsi]
  cmp   al, dl
  je    .found

  cmp   al, NULL_CHAR
  je   .not_found 

  inc   qword [rsp]

  inc   rsi
  jmp   .loop

.not_found:
  mov   rax, -1
  jmp   .return

.found:
  mov   rax, qword [rsp]

.return:
  add   rsp, 0x8
  ret

strpos:
  ; rdi -> string
  ; rsi -> string length
  ; rdx -> substring
  ; rcx -> substring length
  ; return value: rax -> substring position
  sub   rsp, 8 ; string index and substring index
  mov   qword [rsp], 0

  mov   rbx, rcx

  push  rdx
  push  rsi
  push  rdi

  pop   rsi
  pop   rax
  pop   rdi

.loop:
  mov   rcx, rbx
  cld
  rep   cmpsb 
  je    .found

.continue:
  add   qword [rsp], rbx
  sub   qword [rsp], rcx

  cmp   qword [rsp], rax
  jg    .not_found

  add   rdi, rcx
  sub   rdi, rbx

  jmp   .loop

.found:
  mov   rax, qword [rsp]
  jmp   .return

.not_found:
  mov   rax, FAILURE_CODE
  
.return:
  add   rsp, 8
  ret

strlen:
  ; rdi -> *str
  lea   rsi, [rdi]
  mov   rcx, 0

  cmp   rsi, 0
  je    .return

.loop:
  xor   rax, rax
  mov   al, NULL_CHAR
  cmp   byte [rsi], al
  je    .return

  inc   rsi
  inc   rcx

  jmp   .loop

.return:
  mov   rax, rcx
  ret

print:
  ; rdi -> string
  ; rsi -> string_len (optional, if set to 0, need to compute length)
  push  rbp
  mov   rbp, rsp

  mov   rax, rsi
  cmp   rax, 0
  jg    .known_length

  push  rdi
  call  strlen
  test  rax, rax
  jz    .return

  pop   rdi

.known_length:
  lea   rsi, [rdi]
  mov   rdx, rax
  mov   rax, SYS_WRITE
  mov   rdi, STDOUT
  syscall

.return:
  leave
  ret

println:
  push  rbp
  mov   rbp, rsp

  call  print
  cmp   rax, 0
  jl    .return

  push  LINE_FEED

  mov   rax, SYS_WRITE
  lea   rdi, STDOUT
  lea   rsi, [rsp]
  mov   rdx, 1
  syscall

  ; clean stack
  pop   rax
  xor   rax, rax

.return:
  leave  
  ret

; prints a character on standard output
print_char:
  push  rbp
  mov   rbp, rsp

  push  rdi

  mov   rax, SYS_WRITE
  mov   rdi, STDOUT
  lea   rsi, [rsp]
  mov   rdx, 1
  syscall

  leave
  ret

%endif


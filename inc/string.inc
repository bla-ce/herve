section .text
; compare rdi and rsi
; @param  rdi: str
; @param  rsi: substr
; @return rax: rdi == rsi
cmpstr:
  sub   rsp, 0x10

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  call  strlen
  test  rax, rax
  jz    .str_not_equal
  mov   rcx, rax

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  rep   cmpsb
  jne   .str_not_equal

  mov   rax, 1
  jmp   .return

.str_not_equal:
  mov   rax, 0
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; copy the string pointed to by src, into a string at the buffer pointed to by dst
; @param  rdi: dst
; @param  rsi: src
strcpy:
  sub   rsp, 0x10

  cmp   rdi, 0
  jle   .error

  cmp   rdi, 0
  jle   .error

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  lea   rdi, [rsi]
  call  strlen
  mov   rcx, rax

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  rep   movsb

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; converts int in rdi into string
; string pointer needs to be passed
; @param  rdi: int
; @param  rsi: string
; @param  rdx: size
; @return rax: pointer to str(int)
to_string:
  sub   rsp, 0x18

  cmp   rsi, 0
  jle   .error

  ; save parameters onto the stack
  mov   qword [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x10], rdx

  ; go to the end of the buf (print backward)
  mov   rdi, [rsp+0x8]
  add   rdi, qword [rsp+0x10]

  ; go back to let an extra null char at the end
  dec   rdi
  mov   byte [rdi], NULL_CHAR

  dec   rdi
  
  mov   rax, qword [rsp]
  
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
 
  mov   rax, rdi

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; returns the index of the next occurence of the char
; relative to current address
; @param  rdi: string
; @param  rsi: char
; @return rax: index
find_next_char:
  sub   rsp, 0x8
  mov   qword [rsp], 0

  cmp   rdi, 0
  jle   .not_found

  mov   rdx, rsi
  mov   rsi, rdi

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
  mov   rax, FAILURE_CODE
  jmp   .return

.found:
  mov   rax, qword [rsp]

.return:
  add   rsp, 0x8
  ret


; returns the position of a substring inside a string
; @param  rdi: string
; @param  rsi: substring
; @param  rax: return
strpos:
  sub   rsp, 0x28

  cmp   rdi, 0
  jle   .does_not_contain

  cmp   rsi, 0
  jle   .does_not_contain

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  ; get string length
  call  strlen
  mov   qword [rsp+0x10], rax

  cmp   rax, 0
  je    .does_not_contain

  ; get substring length
  mov   rdi, [rsp+0x8]
  call  strlen
  mov   qword [rsp+0x18], rax

  cmp   rax, 0
  je    .does_not_contain

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]

  ; keep track of cursor index
  mov   qword [rsp+0x20], 0

.loop:
  mov   rcx, qword [rsp+0x18]
  rep   cmpsb

  je    .contains

  mov   rbx, qword [rsp+0x18]
  add   qword [rsp+0x20], rbx
  sub   qword [rsp+0x20], rcx

  mov   rbx, qword [rsp+0x20]
  cmp   rbx, qword [rsp+0x10]
  jge   .does_not_contain

  add   rsi, rcx
  sub   rsi, qword [rsp+0x18]

  jmp   .loop

.does_not_contain:
  mov   rax, -1
  jmp   .return

.contains:
  mov   rax, qword [rsp+0x20]

.return:
  add   rsp, 0x28
  ret

; returns the length of the given string
; @param  rdi: string
; @return rax: length of the string
strlen:
  cmp   rdi, 0
  jle   .error

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

.error:
  mov   rax, FAILURE_CODE
  ret

.return:
  mov   rax, rcx
  ret

; prints the string, if length is not passed, it will compute its length
; @param  rdi: sring
; @param  rsi: (optional) string length
print:
  sub   rsp, 0x8

  cmp   rdi, 0
  jle   .error

  mov   [rsp], rdi

  mov   rax, rsi
  cmp   rax, 0
  jg    .known_length

  call  strlen
  test  rax, rax
  jz    .return

  mov   rdi, [rsp]

.known_length:
  lea   rsi, [rdi]
  mov   rdx, rax
  mov   rax, SYS_WRITE
  mov   rdi, STDOUT
  syscall

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; prints the string followed by a new line character
; if length is not passed, it will compute its length
; @param  rdi: sring
; @param  rsi: (optional) string length
println:
  push  rbp
  mov   rbp, rsp

  cmp   rdi, 0
  jle   .error

  call  print
  cmp   rax, 0
  jl    .return

  push  LINE_FEED

  xor   rsi, rsi

  mov   rax, SYS_WRITE
  lea   rdi, STDOUT
  lea   rsi, [rsp]
  mov   rdx, 1
  syscall

  ; clean stack
  pop   rax
  xor   rax, rax

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  leave  
  ret

; prints a character
; @param  rdi: character
print_char:
  push  rbp
  mov   rbp, rsp

  push  rdi

  xor   rsi, rsi

  mov   rax, SYS_WRITE
  mov   rdi, STDOUT
  lea   rsi, [rsp]
  mov   rdx, 1
  syscall

  leave
  ret

; converts string to an integer
; @param  rdi: string
; @return rax: int or -1 if error
stoi:
  sub   rsp, 0x18

  cmp   rdi, 0
  jle   .error

  mov   qword [rsp+0x8], 0
  mov   qword [rsp+0x10], 0

  cmp   byte [rdi], HYPHEN
  jne   .pos_number

  mov   qword [rsp+0x10], 1
  inc   rdi

.pos_number:
  call  strlen
  cmp   rax, 0
  jle   .error

  mov   qword [rsp], rax

.next_char:
  xor   rax, rax
  mov   al, byte [rdi]

  ; make sure char is a digit
  cmp   al, ZERO_CHAR
  jl    .error

  mov   rbx, 0
  cmove rax, rbx
  je    .add

  cmp   al, NINE_CHAR
  jg    .error

  xor   rdx, rdx
  sub   rax, ZERO_CHAR ; get int

  mov   rcx, qword [rsp]
  dec   rcx

  cmp   rcx, 0
  jle   .add

.get_mult:
  mov   rbx, 10
  xor   rdx, rdx
  mul   rbx

  cmp   rcx, 1
  jle   .add

  dec   rcx
  jmp   .get_mult

.add:
  add   qword [rsp+0x8], rax
  mov   r9, qword [rsp+0x8]
  inc   rdi

  dec   qword [rsp]
  cmp   qword [rsp], 0
  je    .return

  jmp   .next_char

.error:
  mov   rax, FAILURE_CODE
  add   rsp, 0x18
  ret

.return:
  mov   rax, qword [rsp+0x8]
  mov   rbx, 0
  sub   rbx, rax

  ; check if it's a neg number
  cmp   qword [rsp+0x10], 1
  cmove rax, rbx

  add   rsp, 0x18
  ret

; returns the int value of ascii character or -1 if invalid
; @param  rdi: ascii
; @return rax: int
ascii_to_int:
  sub   rdi, 0x30

  cmp   rdi, 0x0
  jl    .error

  cmp   rdi, 0x9
  jg    .error

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  mov   rax, rdi
  ret

; prints padded string, if strlen is bigger than padding, add '...' before
; @param  rdi: str
; @param  rsi: padding
; @return rax: void
padding_print:
  sub   rsp, 0x18

  cmp   rdi, 0
  jle   .error

  mov   [rsp], rdi
  mov   qword [rsp+0x8], rsi

  ; get string length
  call  strlen
  mov   qword [rsp+0x10], rax

  cmp   rax, qword [rsp+0x8]
  je    .equal
  jl    .less
  jg    .more

; print the string without padding
.equal:
  mov   rdi, [rsp]
  mov   rsi, qword [rsp+0x8]
  call  print
  jmp   .return

.less:
  ; print string
  mov   rdi, [rsp]
  mov   rsi, qword [rsp+0x10]
  call  print

  ; get difference
  mov   r9, qword [rsp+0x8]
  sub   r9, qword [rsp+0x10]

  ; print padding
.loop:
  mov   rdi, SPACE
  call  print_char

  dec   r9
  
  cmp   r9, 0
  jg    .loop

  jmp   .return 

.more:
  ; get length and subtract 3 for the dots
  mov   rdi, [rsp]
  mov   rsi, qword [rsp+0x8]
  sub   rsi, 3
  call  print

  ; print dots
  mov   r9, 3
.loop_dots:
  mov   rdi, DOT
  call  print_char

  dec   r9

  cmp   r9, 0
  jg    .loop_dots

  jmp   .return

.error:
  mov   rax, FAILURE_CODE
  jle   .error

.return:
  add   rsp, 0x18
  ret


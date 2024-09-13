%ifndef STRING_INC
%define STRING_INC

; converts int into string
to_string:
  ; rdi -> int
  ; return value: rax -> points to buffer
  cmp   rdi, 1000000
  jge   invalid_number

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
  
to_string_loop:
  xor   rdx, rdx
  mov   rcx, 0xA              ; divisor, dividend in rax
  div   rcx                   ; quotient in rax and remainder in edx

  add   rdx, ZERO_CHAR

  mov   byte [rdi], dl

  dec   rdi

  cmp   rax, 0
  jne   to_string_loop

  inc   rdi
 
  lea   rax, [rdi]

  jmp   to_string_return

invalid_number:
  mov   rax, -1

to_string_return:
  ret

; returns the index of the next occurence of the char
; relative to current address
; returns the preserved address into rsi
find_next_char:
  ; rdi -> string
  ; rsi -> char
  ; return value -> rax: index
  push  rsi

  lea   rsi, [rdi]

  pop   rdx

  sub   rsp, 0x8
  mov   qword [rsp], 0

find_next_char_loop:
  mov   al, byte [rsi]
  cmp   al, dl
  je    find_next_char_found

  cmp   al, NULL_CHAR
  je   find_next_char_not_found 

  inc   qword [rsp]

  inc   rsi
  jmp   find_next_char_loop

find_next_char_not_found:
  mov   rax, -1
  jmp   find_next_char_return

find_next_char_found:
  mov   rax, qword [rsp]

find_next_char_return:
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

strpos_loop:
  mov   rcx, rbx
  cld
  rep   cmpsb 
  je    strpos_found

  add   qword [rsp], rbx
  sub   qword [rsp], rcx

  cmp   qword [rsp], rax
  jg    strpos_not_found

  add   rdi, rcx
  sub   rdi, rbx

  jmp   strpos_loop

strpos_found:
  mov   rax, qword [rsp]
  jmp   strpos_return

strpos_not_found:
  mov   rax, -1
  
strpos_return:
  add   rsp, 8
  ret

get_length:
  ; rdi -> *str
  lea   rsi, [rdi]
  mov   rcx, 0

  cmp   rsi, 0
  je    get_length_return

get_length_loop:
  xor   rax, rax
  mov   al, NULL_CHAR
  cmp   byte [rsi], al
  je    get_length_return

  inc   rsi
  inc   rcx

  jmp   get_length_loop

get_length_return:
  mov   rax, rcx
  ret

debug:
  ; rdi -> string
  ; rsi -> string_len (optional, if set to 0, need to compute length)
  mov   rax, rsi
  cmp   rax, 0
  jg    known_length

  push  rdi
  call  get_length

  pop   rdi

known_length:
  lea   rsi, [rdi]
  mov   rdx, rax
  mov   rax, SYS_WRITE
  mov   rdi, STDOUT
  syscall

  ret

%endif


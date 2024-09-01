%ifndef STRING_INC
%define STRING_INC

; converts int into string
to_string:
  ; rdi -> int
  ; rsi -> string
  ; rdx -> string max len

  push  rdi

  lea   rdi, [rsi]
  add   rdi, rdx
  dec   rdi

  ; null char at the end of the string
  mov   rax, NULL_CHAR
  stosb

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

  mov   rax, 0
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

%macro strpos 4
  ; %1 -> string
  ; %2 -> string length
  ; %3 -> substring
  ; %4 -> substring length
  sub   rsp, 8 ; string index and substring index
  mov   qword [rsp], 0

  mov   rsi, %1
  mov   rax, %2
  mov   rdi, %3
  mov   rbx, %4

%%loop:
  mov   rcx, rbx
  cld
  rep   cmpsb 
  je    %%found

  add   qword [rsp], rbx
  sub   qword [rsp], rcx

  cmp   qword [rsp], rax
  jg    %%not_found

  add   rdi, rcx
  sub   rdi, rbx

  jmp   %%loop

%%found:
  mov   rax, qword [rsp]
  jmp   %%return

%%not_found:
  mov   rax, -1
  jmp   %%return
  
%%return:
  add   rsp, 8

%endmacro

get_length:
  ; rdi -> *str
  lea   rsi, [rdi]
  mov   rcx, 0

  cmp   rsi, 0
  je    get_length_return

get_length_loop:
  mov   al, NULL_CHAR
  cmp   byte [rsi], al
  je    get_length_return

  inc   rsi
  inc   rcx

  jmp   get_length_loop

get_length_return:
  mov   rax, rcx
  ret

%endif


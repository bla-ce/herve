%ifndef STRING_INC
%define STRING_INC

; converts int into string
%macro to_string 3
  ; %1 -> int
  ; %2 -> string
  ; %3 -> string max len

  mov   rdi, %2
  add   rdi, %3
  dec   rdi

  ; null char at the end of the string
  mov   rax, NULL_CHAR
  stosb

  sub   rdi, 2

  mov   rax, %1
  
%%loop:
  xor   rdx, rdx
  mov   rcx, 0xA              ; divisor, dividend in rax
  div   rcx                   ; quotient in rax and remainder in edx

  add   rdx, ZERO_CHAR

  mov   byte [rdi], dl

  dec   rdi

  cmp   rax, 0
  jne   %%loop

  inc   rdi
%endmacro

; returns the index of the next occurence of the char
; relative to current address
%macro find_next_char 2
  ; %1 -> string
  ; %2 -> char
  ; return value -> rax: index
  mov   rsi, %1
  mov   rbx, %2

  sub   rsp, 0x8
  mov   qword [rsp], 0

%%next:
  mov   al, byte [rsi]
  cmp   al, bl
  je    %%found

  cmp   al, NULL_CHAR
  je    %%not_found

  inc   qword [rsp]

  inc   rsi
  jmp   %%next

%%not_found:
  mov   rax, -1
  jmp   %%return

%%found:
  mov   rax, qword [rsp]

%%return:
  sub   rsi, qword [rsp]
  add   rsp, 0x8
%endmacro

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

%macro get_length 1
  ; %1 -> str
  mov   rsi, %1
  mov   rcx, 0

  cmp   rsi, 0
  je    %%return

%%loop:
  mov   al, NULL_CHAR
  cmp   byte [rsi], al
  je    %%return

  inc   rsi
  inc   rcx

  jmp   %%loop

%%return:
  mov   rax, rcx
%endmacro

%endif


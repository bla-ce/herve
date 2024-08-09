%ifndef STRING_INC
%define STRING_INC

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


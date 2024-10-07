%ifndef TIME_INC
%define TIME_INC

; return if the year in rdi is a leap year
; @param  rdi: year
; @return rax: (bool)is_leap_year
is_leap_year:
  sub   rsp, 0x8

  ; save year onto the stack
  mov   qword [rsp], rdi

  ; check if year % 4 is equal to 0
  xor   rdx, rdx
  mov   rax, qword [rsp]
  mov   rbx, 0x4
  div   rbx

  cmp   rdx, 0
  je    check_mod_100

  jmp   check_mod_400

check_mod_100:
  ; if yes, check if year % 100 != 0

  xor   rdx, rdx
  mov   rax, qword [rsp]
  mov   rbx, 100
  div   rbx

  cmp   rdx, 0
  je    is_not_leap

  jmp   is_leap

check_mod_400:
  ; check if year % 400 == 0
  xor   rdx, rdx
  mov   rax, qword [rsp]
  mov   rbx, 400
  div   rbx

  cmp   rdx, 0
  jne    is_not_leap

is_leap:
  ; if yes, return 1
  mov   rax, 1
  add   rsp, 0x8
  ret

is_not_leap:
  ; return 0
  mov   rax, 0

  ; clean stack and return
  add   rsp, 0x8
  ret

%endif


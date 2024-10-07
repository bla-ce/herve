%ifndef TIME_INC
%define TIME_INC

; return the number if days in the month
; @param  rdi: month
; @param  rsi: year
; @return rax: n_days
days_in_month:
  sub   rsp, 0x10

  mov   qword [rsp], rdi
  mov   qword [rsp+0x8], rsi

  ; check if month is two
  cmp   qword [rsp], 2
  jne   not_feb

  ; if yes, check if year is leap year
  mov   rdi, qword [rsp+0x8]
  call  is_leap_year

  cmp   rax, 0
  je    not_feb

  ; if yes, return 29
  mov   rax, 29
  jmp   return_days_in_month

not_feb:
  ; return days of (month-1)
  lea   rdi, [days_in_each_month]
  add   rdi, qword [rsp]
  dec   rdi

  xor   rax, rax
  mov   al, byte [rdi]

return_days_in_month:
  ; clean stack
  add   rsp, 0x10
  ret

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


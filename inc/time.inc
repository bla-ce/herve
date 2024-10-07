%ifndef TIME_INC
%define TIME_INC

; convert unix to date
; @param  rdi: timestamp
; @return rax: pointer to dt struct
unix_to_date_time:
  sub   rsp, 0x8

  mov   qword [rsp], rdi

  ; second = timestamp % seconds_in_minute
  xor   rdx, rdx
  mov   rax, qword [rsp]
  mov   rbx, SECONDS_IN_MINUTE
  div   rbx

  mov   qword [dt_second], rdx

  ; timestamp /= seconds in minute
  mov   qword [rsp], rax

  ; minute = timestamp % seconds_in_minute
  xor   rdx, rdx
  mov   rbx, SECONDS_IN_MINUTE
  div   rbx

  mov   qword [dt_minute], rdx

  ; timestamp /= seconds_in_minute
  mov   qword [rsp], rax

  ; hour = timestamp % 24
  xor   rdx, rdx
  mov   rbx, HOURS_IN_DAY
  div   rbx

  mov   qword [dt_hour], rdx

  ; timestamp /= 24
  mov   qword [rsp], rax

  mov   qword [dt_year], EPOCH_YEAR

  ; calculate current year
calc_curr_year:
  ; check if year is leap_year
  mov   rdi, qword [dt_year]
  call  is_leap_year

  ; check if n_days is 366 or 365
  mov   rbx, DAYS_IN_LEAP_YEAR
  mov   rdx, DAYS_IN_YEAR

  cmp   rax, 0
  cmove rbx, rdx

  cmp   qword [rsp], rbx
  jl    calc_curr_month

  ; while loop
  ; sub timestamp 366 or 365 depending on is leap
  sub   qword [rsp], rbx

  ; inc year
  inc   qword [dt_year]

  jmp   calc_curr_year


calc_curr_month:
  mov   qword [dt_month], 1

calc_month_loop:
  ; calculate current month
  ; get n_days in month
  mov   rdi, qword [dt_month]
  mov   rsi, qword [dt_year]
  call  days_in_month

  cmp   qword [rsp], rax
  jl    return_unix_to_date

  sub   qword [rsp], rax
  inc   qword [dt_month]

  jmp   calc_month_loop

return_unix_to_date:
  mov   rax, qword [rsp]
  mov   qword [dt_day], rax

  lea   rax, [dt_year]
  add   rsp, 0x8
  ret

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
  mov   rbx, qword [rsp]
  dec   rbx

  xor   rax, rax
  mov   al, byte [days_in_each_month + rbx]

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


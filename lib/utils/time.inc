section .bss
  ; dt struct
dt_t:
  .year   resq 1
  .month  resq 1
  .day    resq 1
  .hour   resq 1
  .minute resq 1
  .second resq 1
dt_t_end:

timespec_t:
  .tv_sec   resq 1
  .tv_nsec  resq 1
timespec_t_end:

section .data

TIMESPEC_STRUCT_LEN equ timespec_t_end - timespec_t

TIMESPEC_OFF_TV_SEC  equ 0x0
TIMESPEC_OFF_TV_NSEC equ TIMESPEC_OFF_TV_SEC + 0x8

DT_STRUCT_LEN equ dt_t_end - dt_t

; offsets
DT_STRUCT_YEAR    equ 0
DT_STRUCT_MONTH   equ DT_STRUCT_YEAR + 0x8
DT_STRUCT_DAY     equ DT_STRUCT_MONTH + 0x8
DT_STRUCT_HOUR    equ DT_STRUCT_DAY + 0x8
DT_STRUCT_MINUTE  equ DT_STRUCT_HOUR + 0x8
DT_STRUCT_SECOND  equ DT_STRUCT_MINUTE + 0x8

; time constants
CLOCK_REALTIME  equ 0
HOURS_IN_DAY    equ 24
EPOCH_YEAR      equ 1970

SECONDS_IN_MINUTE equ 60
SECONDS_IN_HOUR   equ 3600
SECONDS_IN_DAY    equ 86400
DAYS_IN_YEAR      equ 365
DAYS_IN_LEAP_YEAR equ 366

utc_str db "UTC", NULL_CHAR

DEFAULT_TIME_STR_LEN equ 24 ; yyyy-mm-dd hh:mm:ss UTC\0

days_in_each_month db 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31

time_unit_str:
  .ns db "ns", NULL_CHAR
  .us db MICRO_FIRST, MICRO_SECOND, "s", NULL_CHAR
  .ms db "ms", NULL_CHAR
  .s  db "s", NULL_CHAR

section .text
; convert unix to date
; @param  rdi: timestamp
; @param  rsi: pointer to dt struct
; @return rax: return code
time_unix_to_dt:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> timestamp
  ; [rsp+0x8] -> pointer to the dt struct

  mov   qword [rsp], rdi
  mov   qword [rsp+0x8], rsi

  cmp   rdi, 0
  jl    .error

  cmp   rsi, 0
  jle   .error

  ; second = timestamp % seconds_in_minute
  xor   rdx, rdx
  mov   rax, qword [rsp]
  mov   rbx, SECONDS_IN_MINUTE
  div   rbx

  mov   qword [dt_t.second], rdx

  ; timestamp /= seconds in minute
  mov   qword [rsp], rax

  ; minute = timestamp % seconds_in_minute
  xor   rdx, rdx
  mov   rbx, SECONDS_IN_MINUTE
  div   rbx

  mov   qword [dt_t.minute], rdx

  ; timestamp /= seconds_in_minute
  mov   qword [rsp], rax

  ; hour = timestamp % 24
  xor   rdx, rdx
  mov   rbx, HOURS_IN_DAY
  div   rbx

  mov   qword [dt_t.hour], rdx

  ; timestamp /= 24
  mov   qword [rsp], rax

  mov   qword [dt_t.year], EPOCH_YEAR

  ; calculate current year
.curr_year:
  ; check if year is leap_year
  mov   rdi, qword [dt_t.year]
  call  time_is_leap_year

  ; check if n_days is 366 or 365
  mov   rbx, DAYS_IN_LEAP_YEAR
  mov   rdx, DAYS_IN_YEAR

  cmp   rax, FALSE
  cmove rbx, rdx

  cmp   qword [rsp], rbx
  jl    .curr_month

  ; while loop
  ; sub timestamp 366 or 365 depending on is leap
  sub   qword [rsp], rbx

  ; inc year
  inc   qword [dt_t.year]

  jmp   .curr_year

.curr_month:
  mov   qword [dt_t.month], 0

.loop:
  ; calculate current month
  ; get n_days in month
  mov   rdi, qword [dt_t.month]
  mov   rsi, qword [dt_t.year]
  call  time_days_in_month
  cmp   qword [rsp], rax
  jle   .loop_end

  sub   qword [rsp], rax
  inc   qword [dt_t.month]

  jmp   .loop
.loop_end:
  mov   rax, qword [rsp]
  mov   qword [dt_t.day], rax

  ; populate the struct
  mov   rdi, [rsp+0x8]

  mov   rax, qword [dt_t.year]
  mov   [rdi+DT_STRUCT_YEAR], rax
  mov   rax, qword [dt_t.month]
  mov   [rdi+DT_STRUCT_MONTH], rax
  mov   rax, qword [dt_t.day]
  mov   [rdi+DT_STRUCT_DAY], rax
  mov   rax, qword [dt_t.hour]
  mov   [rdi+DT_STRUCT_HOUR], rax
  mov   rax, qword [dt_t.minute]
  mov   [rdi+DT_STRUCT_MINUTE], rax
  mov   rax, qword [dt_t.second]
  mov   [rdi+DT_STRUCT_SECOND], rax

  ; return
  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; return the number if days in the month
; @param  rdi: month
; @param  rsi: year
; @return rax: n_days
time_days_in_month:
  sub   rsp, 0x10

  mov   qword [rsp], rdi
  mov   qword [rsp+0x8], rsi

  ; check if month is two
  cmp   qword [rsp], 2
  jne   .skip_feb

  ; if yes, check if year is leap year
  mov   rdi, qword [rsp+0x8]
  call  time_is_leap_year
  cmp   rax, FALSE
  je    .skip_feb

  ; if yes, return 29
  mov   rax, 29
  jmp   .return

.skip_feb:
; return days of (month-1)
  mov   rbx, qword [rsp]
  dec   rbx

  xor   rax, rax
  mov   al, byte [days_in_each_month + rbx]

.return:
  ; clean stack
  add   rsp, 0x10
  ret

; return if the year in rdi is a leap year
; @param  rdi: year
; @return rax: (bool)is_leap_year
time_is_leap_year:
  sub   rsp, 0x8

  ; save year onto the stack
  mov   qword [rsp], rdi

  ; check if year % 4 is equal to 0
  xor   rdx, rdx
  mov   rax, qword [rsp]
  mov   rbx, 0x4
  div   rbx

  cmp   rdx, 0
  je    .check_mod_100

  jmp   .check_mod_400

.check_mod_100:
  ; if yes, check if year % 100 != 0

  xor   rdx, rdx
  mov   rax, qword [rsp]
  mov   rbx, 100
  div   rbx

  cmp   rdx, 0
  je    .is_not_leap

  jmp   .is_leap

.check_mod_400:
  ; check if year % 400 == 0
  xor   rdx, rdx
  mov   rax, qword [rsp]
  mov   rbx, 400
  div   rbx

  cmp   rdx, 0
  jne   .is_not_leap

.is_leap:
  ; if yes, return 1
  mov   rax, 1
  add   rsp, 0x8
  ret

.is_not_leap:
  ; return 0
  mov   rax, 0

  ; clean stack and return
  add   rsp, 0x8
  ret

; return now's unix timestamp
; @return rax: timestamp
time_now:
  sub   rsp, TIMESPEC_STRUCT_LEN

  mov   rax, SYS_CLOCK_GETTIME
  mov   rdi, CLOCK_REALTIME
  lea   rsi, [rsp]
  syscall
  cmp   rax, 0
  jl    .error

  lea   rdi, [rsp]
  mov   rax, qword [rdi+TIMESPEC_OFF_TV_SEC]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, TIMESPEC_STRUCT_LEN
  ret

; return the fractional part of the current second in nanoseconds
; @return rax: ns
time_now_ns:
  sub   rsp, TIMESPEC_STRUCT_LEN

  mov   rax, SYS_CLOCK_GETTIME
  mov   rdi, CLOCK_REALTIME
  lea   rsi, [rsp]
  syscall
  cmp   rax, 0
  jl    .error

  lea   rdi, [rsp]
  mov   rax, qword [rdi+TIMESPEC_OFF_TV_NSEC]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, TIMESPEC_STRUCT_LEN
  ret

; convert a dt struct to a string 'yyyy-mm-dd hh:mm:ss UTC'
; @param  rdi: pointer to the dt struct
; @param  rsi: pointer to the string
; @return rax: return code
time_dt_to_str:
  sub   rsp, 0x20

  ; STACK USAGE
  ; [rsp]       -> pointer to dt struct
  ; [rsp+0x8]   -> pointer to the mallocd string
  ; [rsp+0x10]  -> dynamic pointer
  ; [rsp+0x18]  -> tmp string for int to string conversions

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rsi

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   rsi, [rsp]
  mov   rdi, [rsi+DT_STRUCT_YEAR]
  lea   rsi, [rsp+0x18]
  mov   rdx, 5
  call  itoa
  cmp   rax, 0
  jl    .error

  ; copy the year
  mov   rdi, [rsp+0x10]
  mov   rsi, rax
  call  strcpy
  cmp   rax, 0
  jl    .error

  ; add '-'
  mov   rdi, rax
  mov   rax, HYPHEN
  stosb

  mov   [rsp+0x10], rdi

  mov   rsi, [rsp]
  mov   rdi, [rsi+DT_STRUCT_MONTH]
  cmp   rdi, 10
  jge   .no_zero_month

  mov   rdi, [rsp+0x10]
  mov   rax, ZERO_CHAR
  stosb

  mov   [rsp+0x10], rdi

.no_zero_month:
  mov   rsi, [rsp]
  mov   rdi, [rsi+DT_STRUCT_MONTH]
  lea   rsi, [rsp+0x18]
  mov   rdx, 3
  call  itoa
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x10]
  mov   rsi, rax
  call  strcpy
  cmp   rax, 0
  jl    .error

  ; add '-'
  mov   rdi, rax
  mov   rax, HYPHEN
  stosb

  mov   [rsp+0x10], rdi

  mov   rsi, [rsp]
  mov   rdi, [rsi+DT_STRUCT_DAY]
  cmp   rdi, 10
  jge   .no_zero_day

  mov   rdi, [rsp+0x10]
  mov   rax, ZERO_CHAR
  stosb

  mov   [rsp+0x10], rdi

.no_zero_day:
  mov   rsi, [rsp]
  mov   rdi, [rsi+DT_STRUCT_DAY]
  lea   rsi, [rsp+0x18]
  mov   rdx, 3
  call  itoa
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x10]
  mov   rsi, rax
  call  strcpy
  cmp   rax, 0
  jl    .error

  ; add '-'
  mov   rdi, rax
  mov   rax, SPACE
  stosb

  mov   [rsp+0x10], rdi

  mov   rsi, [rsp]
  mov   rdi, [rsi+DT_STRUCT_HOUR]
  cmp   rdi, 10
  jge   .no_zero_hour

  mov   rdi, [rsp+0x10]
  mov   rax, ZERO_CHAR
  stosb

  mov   [rsp+0x10], rdi

.no_zero_hour:
  mov   rsi, [rsp]
  mov   rdi, [rsi+DT_STRUCT_HOUR]
  lea   rsi, [rsp+0x18]
  mov   rdx, 3
  call  itoa
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x10]
  mov   rsi, rax
  call  strcpy
  cmp   rax, 0
  jl    .error

  ; add '-'
  mov   rdi, rax
  mov   rax, COLON
  stosb

  mov   [rsp+0x10], rdi

  mov   rsi, [rsp]
  mov   rdi, [rsi+DT_STRUCT_MINUTE]
  cmp   rdi, 10
  jge   .no_zero_minute

  mov   rdi, [rsp+0x10]
  mov   rax, ZERO_CHAR
  stosb

  mov   [rsp+0x10], rdi

.no_zero_minute:
  mov   rsi, [rsp]
  mov   rdi, [rsi+DT_STRUCT_MINUTE]
  lea   rsi, [rsp+0x18]
  mov   rdx, 3
  call  itoa
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x10]
  mov   rsi, rax
  call  strcpy
  cmp   rax, 0
  jl    .error

  ; add '-'
  mov   rdi, rax
  mov   rax, COLON
  stosb

  mov   [rsp+0x10], rdi

  mov   rsi, [rsp]
  mov   rdi, [rsi+DT_STRUCT_SECOND]
  cmp   rdi, 10
  jge   .no_zero_second

  mov   rdi, [rsp+0x10]
  mov   rax, ZERO_CHAR
  stosb

  mov   [rsp+0x10], rdi

.no_zero_second:
  mov   rsi, [rsp]
  mov   rdi, [rsi+DT_STRUCT_SECOND]
  lea   rsi, [rsp+0x18]
  mov   rdx, 3
  call  itoa
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x10]
  mov   rsi, rax
  call  strcpy
  cmp   rax, 0
  jl    .error

  ; add '-'
  mov   rdi, rax
  mov   rax, SPACE
  stosb

  mov   rsi, utc_str
  mov   rcx, 3
  rep   movsb

  mov   rax, NULL_CHAR
  stosb

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret

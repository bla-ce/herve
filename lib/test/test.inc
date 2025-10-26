section .bss

section .data

assert_str_equal_msg  db "[FAILED]: strings are not equal", NULL_CHAR
assert_equal_msg      db "[FAILED]: values are not equal", NULL_CHAR

assert_str_not_equal_msg  db "[FAILED]: strings are equal", NULL_CHAR
assert_not_equal_msg      db "[FAILED]: values are equal", NULL_CHAR

assert_is_zero_msg      db "[FAILED]: value is not zero", NULL_CHAR
assert_is_not_zero_msg  db "[FAILED]: value is zero", NULL_CHAR

section .text
; tests that first and second values are not equal
; to compare two strings, use assert_string_not_equal
; @param  rdi: value 1
; @param  rsi: value 2
; @return rax: return code
assert_not_equal:
  cmp   rdi, rsi
  je    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rdi, assert_not_equal_msg
  call  println

  mov   rdi, FAILURE_CODE
  call  exit

.return:
  ret

; tests that first and second values are equal
; to compare two strings, use assert_string_equal
; @param  rdi: value 1
; @param  rsi: value 2
; @return rax: return code
assert_equal:
  cmp   rdi, rsi
  jne   .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rdi, assert_equal_msg
  call  println

  mov   rdi, FAILURE_CODE
  call  exit

.return:
  ret

; tests that first and second strings are equal
; @param  rdi: string 1
; @param  rsi: string 2
; @return rax: return code
assert_string_equal:
  sub   rsp, 0x10

  ; *** STACK USAGE *** ;
  ; [rsp]     -> pointer to the first string
  ; [rsp+0x8] -> pointer to the second string

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  call  strcmp
  cmp   rax, 0
  jl    .error

  cmp   rax, TRUE
  jne   .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rdi, assert_str_equal_msg
  call  println

  mov   rdi, [rsp]
  call  println

  mov   rdi, [rsp+0x8]
  call  println


  mov   rdi, FAILURE_CODE
  call  exit

.return:
  add   rsp, 0x10
  ret

; tests that first and second strings are not equal
; @param  rdi: string 1
; @param  rsi: string 2
; @return rax: return code
assert_string_not_equal:
  sub   rsp, 0x10

  ; *** STACK USAGE *** ;
  ; [rsp]     -> pointer to the first string
  ; [rsp+0x8] -> pointer to the second string

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  call  strcmp
  cmp   rax, 0
  jl    .error

  cmp   rax, TRUE
  je    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rdi, assert_str_not_equal_msg
  call  println

  mov   rdi, [rsp]
  call  println

  mov   rdi, [rsp+0x8]
  call  println

  mov   rdi, FAILURE_CODE
  call  exit

.return:
  ret

; tests that the value is 0
; @param  rdi: value
; @return rax: return code
assert_is_zero:
  cmp   rdi, 0
  jne   .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rdi, assert_is_zero_msg
  call  println

  mov   rdi, FAILURE_CODE
  call  exit

.return:
  ret

; tests that the value is NOT 0
; @param  rdi: value
; @return rax: return code
assert_is_not_zero:
  cmp   rdi, 0
  je    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rdi, assert_is_not_zero_msg
  call  println

  mov   rdi, FAILURE_CODE
  call  exit

.return:
  ret

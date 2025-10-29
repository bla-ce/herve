global _start

%include "malloc_utils.inc"
%include "malloc.inc"

section .text
_start:
  ; Test cases
  mov   rdi, 0
  call  get_bin ; Expected: 0
  cmp   rax, 0
  jne   .error

  mov	  rdi, 4
  call  get_bin ; Expected: 0
  cmp	  rax, 0
  jne   .error

  mov	  rdi, 8
  call  get_bin ; Expected: 0
  cmp	  rax, 0
  jne   .error

  mov	  rdi, 9
  call  get_bin ; Expected: 0
  cmp	  rax, 0
  jne   .error

  mov	  rdi, 15
  call  get_bin ; Expected: 0
  cmp	  rax, 0
  jne   .error

  mov	  rdi, 16
  call  get_bin ; Expected: 1
  cmp	  rax, 1
  jne   .error

  mov	  rdi, 24
  call  get_bin ; Expected: 1
  cmp	  rax, 1
  jne   .error

  mov	  rdi, 31
  call  get_bin ; Expected: 1
  cmp	  rax, 1
  jne   .error

  mov	  rdi, 32
  call  get_bin ; Expected: 2
  cmp	  rax, 2
  jne   .error

  mov	  rdi, 48
  call  get_bin ; Expected: 2
  cmp	  rax, 2
  jne   .error

  mov	  rdi, 63
  call  get_bin ; Expected: 2
  cmp	  rax, 2
  jne   .error

  mov	  rdi, 64
  call  get_bin ; Expected: 3
  cmp	  rax, 3
  jne   .error

  mov	  rdi, 100
  call  get_bin ; Expected: 3
  cmp	  rax, 3
  jne   .error

  mov	  rdi, 127
  call  get_bin ; Expected: 3
  cmp	  rax, 3
  jne   .error

  mov	  rdi, 128
  call  get_bin ; Expected: 4
  cmp	  rax, 4
  jne   .error

  mov	  rdi, 200
  call  get_bin ; Expected: 4
  cmp	  rax, 4
  jne   .error

  mov	  rdi, 255
  call  get_bin ; Expected: 4
  cmp	  rax, 4
  jne   .error

  mov	  rdi, 256
  call  get_bin ; Expected: 5
  cmp	  rax, 5
  jne   .error

  mov	  rdi, 400
  call  get_bin ; Expected: 5
  cmp	  rax, 5
  jne   .error

  mov	  rdi, 511
  call  get_bin ; Expected: 5
  cmp	  rax, 5
  jne   .error

  mov	  rdi, 512
  call  get_bin ; Expected: 6
  cmp	  rax, 6
  jne   .error

  mov	  rdi, 800
  call  get_bin ; Expected: 6
  cmp	  rax, 6
  jne   .error

  mov	  rdi, 1023
  call  get_bin ; Expected: 6
  cmp	  rax, 6
  jne   .error

  mov	  rdi, 1024
  call  get_bin ; Expected: 7
  cmp	  rax, 7
  jne   .error

  mov	  rdi, 1500
  call  get_bin ; Expected: 7
  cmp	  rax, 7
  jne   .error

  mov	  rdi, 2047
  call  get_bin ; Expected: 7
  cmp	  rax, 7
  jne   .error

  mov	  rdi, 2048
  call  get_bin ; Expected: 8
  cmp	  rax, 8
  jne   .error

  mov	  rdi, 3000
  call  get_bin ; Expected: 8
  cmp	  rax, 8
  jne   .error

  mov	  rdi, 4096
  call  get_bin ; Expected: 9
  cmp	  rax, 9
  jne   .error

  mov	  rdi, 5000
  call  get_bin ; Expected: 9
  cmp	  rax, 9
  jne   .error

  mov	  rdi, 8000
  call  get_bin ; Expected: 9
  cmp	  rax, 9
  jne   .error

  mov	  rdi, 12000
  call  get_bin ; Expected: 10
  cmp	  rax, 10
  jne   .error

  ; If all tests passed, exit with success code
.exit:
  mov		rax, SYS_EXIT
  mov		rdi, 0
  syscall

.error:
  mov		rax, SYS_EXIT
  mov		rdi, FAILURE_CODE
  syscall

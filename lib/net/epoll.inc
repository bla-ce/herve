section .bss

epoll_event_t:
  .events resd 1
  .data   resq 1
epoll_event_t_end:

section .data

EPOLL_EVENT_STRUCT_LEN equ epoll_event_t_end - epoll_event_t

EPOLL_EVENT_OFF_EVENT equ 0x0
EPOLL_EVENT_OFF_DATA  equ EPOLL_EVENT_OFF_EVENT + 0x4

MAX_EVENTS equ 32

EPOLL_CTL_ADD equ 1
EPOLL_CTL_DEL equ 2
EPOLL_CTL_MOD equ 3

EPOLLIN     equ 0x001
EPOLLPRI    equ 0x002
EPOLLOUT    equ 0x004
EPOLLRDNORM equ 0x040
EPOLLNVAL   equ 0x020
EPOLLRDBAND equ 0x080
EPOLLWRNORM equ 0x100
EPOLLWRBAND equ 0x200
EPOLLMSG    equ 0x400
EPOLLERR    equ 0x008
EPOLLHUP    equ 0x010
EPOLLRDHUP  equ 0x2000
EPOLLET     equ 0x80000000

section .text
; removes a file descriptor from the epoll instance interest list
; @param  rdi: file descriptor of the epoll instance
; @param  rsi: file descriptor of the new connection
; @return rax: return code
epoll_remove_fd_from_instance:
  sub   rsp, 0x10
  sub   rsp, EPOLL_EVENT_STRUCT_LEN

  ; *** STACK USAGE  *** ;
  ; [rsp]       -> file descriptor of the epoll instance
  ; [rsp+0x8]   -> file descriptor of the new connection
  ; [rsp+0x10]  -> epoll event struct

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  cmp   rdi, 0
  jl    .error

  cmp   rsi, 0
  jl    .error

  lea   rax, [rsp+0x10]

  mov   qword [rax+EPOLL_EVENT_OFF_EVENT], 0

  mov   rbx, [rsp+0x8]
  mov   [rax+EPOLL_EVENT_OFF_DATA], rbx

  mov   rdi, [rsp]
  mov   rsi, EPOLL_CTL_DEL
  mov   rdx, [rsp+0x8]
  lea   rcx, [rsp+0x10]
  call  epoll_ctl
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  add   rsp, EPOLL_EVENT_STRUCT_LEN
  ret

; adds a file descriptor to the epoll instance interest list
; @param  rdi: file descriptor of the epoll instance
; @param  rsi: file descriptor of the new connection
; @return rax: return code
epoll_add_fd_to_instance:
  sub   rsp, 0x10
  sub   rsp, EPOLL_EVENT_STRUCT_LEN

  ; *** STACK USAGE  *** ;
  ; [rsp]       -> file descriptor of the epoll instance
  ; [rsp+0x8]   -> file descriptor of the new connection
  ; [rsp+0x10]  -> epoll event struct

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  cmp   rdi, 0
  jl    .error

  cmp   rsi, 0
  jl    .error

  lea   rax, [rsp+0x10]

  mov   rbx, EPOLLIN
  mov   rcx, EPOLLET
  or    rbx, rcx
  mov   [rax+EPOLL_EVENT_OFF_EVENT], rbx

  mov   rbx, [rsp+0x8]
  mov   [rax+EPOLL_EVENT_OFF_DATA], rbx

  mov   rdi, [rsp]
  mov   rsi, EPOLL_CTL_ADD
  mov   rdx, [rsp+0x8]
  lea   rcx, [rsp+0x10]
  call  epoll_ctl
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  add   rsp, EPOLL_EVENT_STRUCT_LEN
  ret

; creates an epoll instance and adds the listening socket to its
; interest list
; @param  rdi: file descriptor of the listening socket
; @return rax: file descriptor of the epoll instance
epoll_init:
  sub   rsp, 0x10
  sub   rsp, EPOLL_EVENT_STRUCT_LEN

  ; STACK USAGE
  ; [rsp]     -> file descriptor of the listening socket
  ; [rsp+0x8] -> file descriptor of the epoll instance
  ; [rsp+0x8] -> pointer to the epoll event struct

  mov   [rsp], rdi

  cmp   rdi, 0
  jl    .error

  ; creates epoll instance
  mov   rdi, NO_ARG
  call  epoll_create1
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; populate event struct
  lea   rdi, [rsp+0x10]
  mov   dword [rdi+EPOLL_EVENT_OFF_EVENT], EPOLLIN

  mov   rax, [rsp]
  mov   [rdi+EPOLL_EVENT_OFF_DATA], rax

  ; add listening socket to epoll instance interest list
  mov   rdi, [rsp+0x8]
  mov   rsi, EPOLL_CTL_ADD
  mov   rdx, [rsp]
  lea   rcx, [rsp+0x10]
  call  epoll_ctl
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x8]
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, EPOLL_EVENT_STRUCT_LEN
  add   rsp, 0x10
  ret

; waits for events on the epoll instance
; @param  rdi: file descriptor of the epoll instance
; @param  rsi: pointer to array of events
; @param  rdx: max events to wait for
; @param  rcx: timeout
; @return rax: number of events
epoll_wait:
  mov   rax, SYS_EPOLL_WAIT
  mov   r10, rcx
  syscall

  ret

; creates a new epoll instance
; @param  rdi: flags
; @return rax: file descriptor of the epoll instance
epoll_create1:
  mov   rax, SYS_EPOLL_CREATE1
  syscall

  ret

; add, modify, or remove entries in the interest list of the epoll
; instance
; @param  rdi: file descriptor of the epoll instance
; @param  rsi: operation to perform
; @param  rdx: target file descriptor
; @param  rcx: pointer to event struct
; @return rax: return code
epoll_ctl:
  mov   rax, SYS_EPOLL_CTL
  mov   r10, rcx
  syscall

  ret

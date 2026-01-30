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

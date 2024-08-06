global  _start

%include "server.inc"

section .text
_start:
  server_init 1337
  mov   qword [sockfd], rax

  add_route index_route, index_route_len

  run_server qword [sockfd]

  shutdown

section .data
  sockfd    dq 0

  index_route      db "/index", NULL_CHAR
  index_route_len  equ $ - index_route

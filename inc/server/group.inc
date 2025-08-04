section .bss

group_struct:
  group_routes      resq 1
  group_middlewares resq 1
  group_prefix      resq 1
  group_is_root     resq 1
  group_next        resq 1
group_struct_end:

section .data

GRP_STRUCT_LEN equ group_struct_end - group_struct

; offsets
GRP_STRUCT_OFF_ROUTES       equ 0x0
GRP_STRUCT_OFF_MIDDLEWARES  equ GRP_STRUCT_OFF_ROUTES + 0x8
GRP_STRUCT_OFF_PREFIX       equ GRP_STRUCT_OFF_MIDDLEWARES + 0x8
GRP_STRUCT_OFF_IS_ROOT      equ GRP_STRUCT_OFF_PREFIX + 0x8
GRP_STRUCT_OFF_NEXT         equ GRP_STRUCT_OFF_IS_ROOT + 0x8

section .text

; creates a group and adds it to the server struct
; @param  rdi: pointer to the server struct
; @param  rsi: pointer to the prefix
; @param  rdx: group is root (boolean)
; @return rax: pointer to the group struct
add_group:
  sub   rsp, 0x20

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the server struct
  ; [rsp+0x8]   -> pointer to the prefix
  ; [rsp+0x10]  -> group is root
  ; [rsp+0x18]  -> pointer to the group

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   qword [rsp+0x18], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   rdi, GRP_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  mov   rsi, [rsp+0x8]
  mov   rdx, [rsp+0x10]

  mov   qword [rax+GRP_STRUCT_OFF_ROUTES], 0
  mov   qword [rax+GRP_STRUCT_OFF_MIDDLEWARES], 0
  mov   [rax+GRP_STRUCT_OFF_PREFIX], rsi
  mov   [rax+GRP_STRUCT_OFF_IS_ROOT], rdx
  mov   qword [rax+GRP_STRUCT_OFF_NEXT], 0

  ; TODO: add the group to the server

  jmp   .return

.error:
  mov   rdi, [rsp+0x18]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x20
  ret


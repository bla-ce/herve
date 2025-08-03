section .bss

group_struct:
  routes        resq 1
  middlewares   resq 1
  prefix        resq 1
  is_root       resq 1
group_struct_end:

section .data

GRP_STRUCT_LEN equ group_struct_end - group_struct

; offsets
GRP_STRUCT_OFF_ROUTES       equ 0x0
GRP_STRUCT_OFF_MIDDLEWARES  equ GRP_STRUCT_OFF_ROUTES + 0x8
GRP_STRUCT_OFF_PREFIX       equ GRP_STRUCT_OFF_MIDDLEWARES + 0x8
GRP_STRUCT_OFF_IS_ROOT      equ GRP_STRUCT_OFF_PREFIX + 0x8



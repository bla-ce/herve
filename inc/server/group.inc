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

ROOT_GRP_PREFIX db "/", NULL_CHAR

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

  ; make sure prefix starts with a slash
  mov   rdi, [rsp+0x8]
  xor   rax, rax
  lodsb

  cmp   rax, SLASH
  jne   .error  ; handling errors as a professional herve :)

  ; make sure a group with the same prefix does not already exist
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  call  group_exist
  cmp   rax, 0
  jl    .error

  cmp   rax, TRUE
  je    .error

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

  ; add the group to the server
  mov   rdi, [rsp]
  mov   rsi, rax
  call  add_group_struct_to_server
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x18]

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

; adds the group to the group linked list of the server
; @param  rdi: pointer to the server struct
; @param  rsi: pointer to the group struct
; @return rax: return code
add_group_struct_to_server:
  sub   rsp, 0x10

  ; *** STACK USAGE *** ;
  ; [rsp]     -> pointer to the server struct
  ; [rsp+0x8] -> pointer to the group struct

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  ; get group offset to check if groups are init
  add   rdi, SERVER_OFF_GROUPS

  cmp   qword [rdi], 0
  je    .add

  ; load first group
  mov   rsi, [rdi]
  mov   rdi, rsi
  
.loop:
  ; rdi points to the address of current group
  ; go to next offset
  add   rdi, GRP_STRUCT_OFF_NEXT
  cmp   qword [rdi], 0
  je    .add
 
  ; rdi contains the address of the group
  mov   rsi, [rdi]
  mov   rdi, rsi

  jmp   .loop
.loop_end:

.add:
  mov   rsi, [rsp+0x8]
  mov   [rdi], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; checks that a group with the same prefix or sub prefix does not
; already exist.
; @param  rdi: pointer to the server struct
; @param  rsi: pointer to the group struct
; @return rax: group does exist (bool)
group_exist:
  sub   rsp, 0x18

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the server struct
  ; [rsp+0x8]   -> pointer to the group prefix
  ; [rsp+0x10]  -> pointer to groups linked list

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; load server routes
  mov   rdi, [rsp]
  call  get_server_groups
  cmp   rax, 0
  jl    .error
  je    .loop_end

  ; save group address
  mov   [rsp+0x10], rax

.loop:
  ; check that the prefix does not already exist
  mov   rdi, [rsp+0x10]
  call  get_group_prefix
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  strcmp
  cmp   rax, 0
  jl    .error

  cmp   rax, TRUE
  je    .equal

  ; go to next
.not_equal:
  mov   rdi, [rsp+0x10]
  mov   rsi, [rdi+GRP_STRUCT_OFF_NEXT]

  test  rsi, rsi
  jz    .loop_end

  mov   [rsp+0x10], rsi

  jmp   .loop

.loop_end:
  mov   rax, FALSE
  jmp   .return

.equal:
  mov   rax, TRUE
  jmp   .return
  
.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

; returns the prefix of the group
; @param  rdi: pointer to the group struct
; @return rax: pointer to the prefix
get_group_prefix:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+GRP_STRUCT_OFF_PREFIX]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the root group of the server
; @param  rdi: pointer to the server
; @return rax: pointer to the root group
get_server_root_group:
  sub   rsp, 0x8

  ; *** STACK USAGE *** ;
  ; [rsp] -> pointer to the server struct

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

  ; get group offset to check if groups are init
  add   rdi, SERVER_OFF_GROUPS

  cmp   qword [rdi], 0
  je    .error

  ; load first group
  mov   rsi, [rdi]
  mov   rdi, rsi
  
.loop:
  ; rsi points to the address of current group
  cmp   qword [rsi+GRP_STRUCT_OFF_IS_ROOT], TRUE
  je    .loop_end

  ; go to next offset
  add   rdi, GRP_STRUCT_OFF_NEXT
  cmp   qword [rdi], 0
  je    .error
 
  ; rdi contains the address of the group
  mov   rsi, [rdi]
  mov   rdi, rsi

  jmp   .loop
.loop_end:

  mov   rax, rsi

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret


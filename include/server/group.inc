section .bss

group_t:
  .routes       resq 1
  .middlewares  resq 1
  .prefix       resq 1
  .is_root      resq 1
  .active       resq 1
  .server       resq 1
  .next         resq 1
group_t_end:

section .data

GRP_STRUCT_LEN equ group_t_end - group_t

; offsets
GRP_OFF_ROUTES      equ 0x0
GRP_OFF_MIDDLEWARES equ GRP_OFF_ROUTES + 0x8
GRP_OFF_PREFIX      equ GRP_OFF_MIDDLEWARES + 0x8
GRP_OFF_IS_ROOT     equ GRP_OFF_PREFIX + 0x8
GRP_OFF_ACTIVE      equ GRP_OFF_IS_ROOT + 0x8
GRP_OFF_SERVER      equ GRP_OFF_ACTIVE + 0x8
GRP_OFF_NEXT        equ GRP_OFF_SERVER + 0x8

ROOT_GRP_PREFIX db "/", NULL_CHAR

section .text
; creates a group and adds it to the server struct
; @param  rdi: pointer to the server struct
; @param  rsi: pointer to the prefix
; @param  rdx: group is root (boolean)
; @return rax: pointer to the group struct
group_create:
  sub   rsp, 0x28

  ; STACK USAGE
  ; [rsp]       -> pointer to the server struct
  ; [rsp+0x8]   -> pointer to the prefix
  ; [rsp+0x10]  -> group is root
  ; [rsp+0x18]  -> pointer to the duplicated prefix
  ; [rsp+0x20]  -> pointer to the group

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   qword [rsp+0x18], 0
  mov   qword [rsp+0x20], 0

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

  ; duplicate the prefix
  mov   rdi, [rsp+0x8]
  call  strdup
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  mov   rdi, GRP_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  mov   qword [rax+GRP_OFF_ROUTES], 0
  mov   qword [rax+GRP_OFF_MIDDLEWARES], 0
  mov   qword [rax+GRP_OFF_ACTIVE], TRUE
  mov   qword [rax+GRP_OFF_NEXT], 0

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x18]
  mov   rdx, [rsp+0x10]

  mov   [rax+GRP_OFF_SERVER], rdi
  mov   [rax+GRP_OFF_PREFIX], rsi
  mov   [rax+GRP_OFF_IS_ROOT], rdx

  ; add the group to the server
  mov   rdi, [rsp]
  add   rdi, SERVER_OFF_GROUPS
  mov   rsi, rax
  mov   rdx, GRP_OFF_NEXT
  call  linked_list_add_entry
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x20]

  jmp   .return

.error:
  mov   rdi, [rsp+0x18]
  test  rdi, rdi
  jz    .free_group

  call  free

.free_group:
  mov   rdi, [rsp+0x20]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x28
  ret

; checks that a group with the same prefix or sub prefix does not
; already exist.
; @param  rdi: pointer to the server struct
; @param  rsi: pointer to the group struct
; @return rax: group does exist (bool)
group_exist:
  sub   rsp, 0x18

  ; STACK USAGE
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
  call  server_get_groups
  cmp   rax, 0
  jl    .error
  je    .loop_end

  ; save group address
  mov   [rsp+0x10], rax

.loop:
  ; check that the prefix does not already exist
  mov   rdi, [rsp+0x10]
  call  group_get_prefix
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
  mov   rsi, [rdi+GRP_OFF_NEXT]

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

; returns the middlewares of the group
; @param  rdi: pointer to the group struct
; @return rax: pointer to the middlewares linked list
group_get_middleware:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+GRP_OFF_MIDDLEWARES]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the prefix of the group
; @param  rdi: pointer to the group struct
; @return rax: pointer to the prefix
group_get_prefix:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+GRP_OFF_PREFIX]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the routes ll of the group
; @param  rdi: pointer to the group struct
; @param  rsi: pointer to the routes ll
; @return rax: return code
group_set_routes:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   [rdi+GRP_OFF_ROUTES], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the routes ll of the group
; @param  rdi: pointer to the group struct
; @return rax: pointer to the routes struct
group_get_routes:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+GRP_OFF_ROUTES]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the server of the group
; @param  rdi: pointer to the group struct
; @param  rsi: pointer to the server struct
; @return rax: return code
group_set_server:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   [rdi+GRP_OFF_SERVER], rsi

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the server of the group
; @param  rdi: pointer to the group struct
; @return rax: pointer to the server struct
group_get_server:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+GRP_OFF_SERVER]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the group struct corresponsing to the url of the ctx
; if two or more groups match, the first one will be chosen.
; @param  rdi: pointer to the server struct
; @param  rsi: pointer to the context struct
; @return rax: pointer to the group struct or 0 if no match
get_group_from_ctx:
  sub   rsp, 0x28

  ; STACK USAGE
  ; [rsp]       -> pointer to the server struct
  ; [rsp+0x8]   -> pointer to the context struct
  ; [rsp+0x10]  -> pointer to the groups linked list
  ; [rsp+0x18]  -> pointer to the url

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  cmp   rdi, 0
  jle   .error

  ; get the groups from the server
  mov   rdi, [rsp]
  call  server_get_groups
  cmp   rax, 0
  jle   .error

  mov   [rsp+0x10], rax

  ; get the url from the context request
  mov   rdi, [rsp+0x8]
  call  ctx_get_request
  cmp   rax, 0
  jle   .error

  mov   rdi, rax
  call  get_request_url
  cmp   rax, 0
  jle   .error

  mov   [rsp+0x18], rax

.loop:
  mov   rdi, [rsp+0x10]
  cmp   qword [rdi+GRP_OFF_IS_ROOT], TRUE
  je    .skip_root

  ; get the prefix of the group
  mov   rdi, [rsp+0x10]
  call  group_get_prefix
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  ; check if the url starts with the prefix of the group
  mov   rdi, [rsp+0x18]
  mov   rsi, [rsp+0x20]
  call  starts_with
  cmp   rax, 0
  jl    .error

  cmp   rax, TRUE
  jne   .skip_root

  ; check if the last character of the url is '/' or '\0'
  mov   rdi, [rsp+0x20]
  call  strlen
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x18]
  add   rdi, rax

  cmp   byte [rdi], SLASH
  je    .loop_end

  cmp   byte [rdi], NULL_CHAR
  je    .loop_end

.skip_root:
  mov   rdi, [rsp+0x10]

  ; go to next offset
  add   rdi, GRP_OFF_NEXT
  cmp   qword [rdi], 0
  je    .error

  ; rdi contains the address of the group
  mov   rsi, [rdi]

  mov   [rsp+0x10], rsi

  jmp   .loop

.loop_end:
  mov   rax, [rsp+0x10]

  jmp   .return

.error:
  xor   rax, rax   ; no need to return -1

.return:
  add   rsp, 0x28
  ret

; frees the group struct and all associated resources
; @param  rdi: pointer to the group struct
; @return rax: return code
group_free:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> pointer to the group struct
  ; [rsp+0x8] -> middleware next pointer

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

  ; free the middlewares
  mov   rsi, [rsp]
  add   rsi, GRP_OFF_MIDDLEWARES

  cmp   qword [rsi], 0
  je    .loop_end

.loop:
  mov   rdi, [rsi]    ; rdi points to the middleware struct

  ; save next
  lea   rsi, [rdi+MIDDLEWARE_OFF_NEXT]
  mov   [rsp+0x8], rsi

  ; free the middleware
  call  free
  cmp   rax, 0
  jl    .error

  ; go to next
  mov   rsi, [rsp+0x8]

  cmp   qword [rsi], 0
  je    .loop_end

  jmp   .loop
.loop_end:
  ; remove group from the server
  mov   rdi, [rsp]
  call  group_get_server
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  add   rdi, SERVER_OFF_GROUPS
  mov   rsi, [rsp]
  mov   rdx, GRP_OFF_NEXT
  call  linked_list_remove_entry
  cmp   rax, 0
  jl    .error

  ; remove all the routes from the server route
  mov   rdi, [rsp]
  call  group_free_routes
  cmp   rax, 0
  jl    .error

  ; free the prefix
  mov   rdi, [rsp]
  call  group_get_prefix
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  free
  cmp   rax, 0
  jl    .error

  ; free the struct
  mov   rdi, [rsp]
  call  free
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; removes the route from the server and frees them
; @param  rdi: pointer to the group
; @return rax: return code
group_free_routes:
  sub   rsp, 0x8

  ; STACK USAGE
  ; [rsp]       -> pointer to the group

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

  ; get routes ll
  mov   rdi, [rsp]
  add   rdi, GRP_OFF_ROUTES
  mov   rsi, route_free
  mov   rdx, ROUTE_OFF_GRP_NEXT
  call  linked_list_free
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; sets the active property to false
; @param  rdi: pointer to the group struct
; @return rax: return code
group_deactivate:
  cmp   rdi, 0
  jle   .error

  mov   qword [rdi+GRP_OFF_ACTIVE], FALSE

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the active property to true
; @param  rdi: pointer to the group struct
; @return rax: return code
group_activate:
  cmp   rdi, 0
  jle   .error

  mov   qword [rdi+GRP_OFF_ACTIVE], TRUE

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

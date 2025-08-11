section .bss
route_struct:
  route_method    resq 1
  route_url       resq 1
  route_callback  resq 1
  route_group     resq 1
  route_next      resq 1
route_struct_end:

section .data
  ROUTE_STRUCT_LEN equ route_struct_end - route_struct

  ROUTE_OFF_METHOD    equ 0x0
  ROUTE_OFF_URL       equ ROUTE_OFF_METHOD + 0x8
  ROUTE_OFF_CALLBACK  equ ROUTE_OFF_URL + 0x8
  ROUTE_OFF_GROUP     equ ROUTE_OFF_CALLBACK + 0x8
  ROUTE_OFF_NEXT      equ ROUTE_OFF_GROUP + 0x8

  log_pre_routes  db SPACE, SPACE, NULL_CHAR

section .text
; logs the route "METHOD  URL"
; @param  rdi: pointer to the route
; @return rax: return code
log_route:
  sub   rsp, 0x8

  ; *** STACK USAGE *** ;
  ; [rsp] -> pointer to the route

  mov   [rsp], rdi

  cmp   rdi, 0
  jle   .error

  ; print tab
  mov   rdi, log_pre_routes
  call  print
  cmp   rax, 0
  jl    .error

  ; print method
  mov   rsi, [rsp]
  mov   rdi, [rsi+ROUTE_OFF_METHOD]
  call  print
  cmp   rax, 0
  jl    .error

  mov   rdi, HORIZONTAL_TAB
  call  print_char
  cmp   rax, 0
  jl    .error

  ; print url
  mov   rsi, [rsp]
  mov   rdi, [rsi+ROUTE_OFF_URL]
  call  println
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

; returns the group associated to the route struct
; @param  rdi: pointer to route struct
; @return rax: group address
get_route_group:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+ROUTE_OFF_GROUP]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the url associated to the route struct
; @param  rdi: pointer to route struct
; @return rax: url address
get_route_url:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+ROUTE_OFF_URL]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the method associated to the route struct
; @param  rdi: pointer to route struct
; @return rax: method address
get_route_method:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+ROUTE_OFF_METHOD]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the callback associated to the route struct
; @param  rdi: pointer to route struct
; @return rax: callback address
get_route_callback:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+ROUTE_OFF_CALLBACK]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; compares the route with the url and the method provided in args
; @param  rdi: pointer to request struct
; @param  rsi: pointer to route
; @param  rax: (bool) route corresponds to the request
compare_route:
  sub   rsp, 0x30

  cmp   rdi, 0
  jl    .error

  cmp   rsi, 0
  jl    .error

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to request struct
  ; [rsp+0x8]   -> pointer to server route
  ; [rsp+0x10]  -> pointer to request url
  ; [rsp+0x18]  -> '?' position if query parameters
  ; [rsp+0x20]  -> pointer to request url without query parameters
  ; [rsp+0x30]  -> result

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  ; init to 0 -> this is for query parameters
  mov   qword [rsp+0x20], 0

  ; compare methods
  mov   rsi, [rsp]
  mov   rdi, [rsi+REQ_OFF_METHOD] 
  mov   rdx, [rsp+0x8]
  mov   rsi, [rdx+ROUTE_OFF_METHOD]
  call  strcmp

  cmp   rax, 0
  jl    .error
  je    .not_the_same

  ; compare urls
  mov   rsi, [rsp]
  mov   rdi, [rsi+REQ_OFF_URL] 

  ; save url
  mov   [rsp+0x10], rdi

  ; check if there is any query parameters
  mov   rdi, [rsp+0x10]
  mov   rsi, QUESTION_MARK
  call  find_next_char

  cmp   rax, 0
  jl    .no_query

  ; save position
  mov   qword [rsp+0x18], rax

  ; malloc new url without query parameters
  mov   rdi, [rsp+0x10]
  mov   rsi, rax
  call  strndup

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  mov   rdi, [rsp+0x20]
  mov   [rsp+0x10], rdi

.no_query:
  ; check if compared route is a wild card
  mov   rsi, [rsp+0x8]
  mov   rdi, [rsi+ROUTE_OFF_URL]
  call  is_wildcard_route
  cmp   rax, TRUE
  je    .cmp_wildcard

  mov   rdi, [rsp+0x10]
  mov   rdx, [rsp+0x8]
  mov   rsi, [rdx+ROUTE_OFF_URL]
  call  strcmp

  cmp   rax, 0
  jl    .error
  je    .not_the_same

  jmp   .equal

.cmp_wildcard:
  mov   rsi, [rsp+0x8] 
  mov   rdi, [rsi+ROUTE_OFF_URL]
  mov   rsi, ASTERISK
  call  find_next_char
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x10]
  mov   rdx, [rsp+0x8]
  mov   rsi, [rdx+ROUTE_OFF_URL]
  mov   rdx, rax
  call  strncmp
  cmp   rax, 0
  jl    .error
  je    .not_the_same

.equal:
  mov   qword [rsp+0x28], TRUE

  jmp   .return

.not_the_same:
  mov   qword [rsp+0x28], FALSE

  jmp   .return

.error:
  mov   qword [rsp+0x28], FAILURE_CODE

.return:
  ; free duplicated route without query
  mov   rdi, [rsp+0x20]
  test  rdi, rdi
  jz    .no_free
  
  call  free

.no_free:
  mov   rax, qword [rsp+0x28]

  add   rsp, 0x30
  ret

; create a route serving files located in a specified dir 
; @param  rdi: pointer to server struct
; @param  rsi: path
; @param  rdx: (bool) recursion
; @return rax: code
add_dir_route:
  sub   rsp, 0x48

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   [rsp+0x8], rsi
  mov   qword [rsp+0x38], rdx
  mov   [rsp+0x40], rdi

  ; map file_path and direntries
  mov   rdi, DIR_ENTRIES_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x28], rax

  mov   rdi, FILE_PATH_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x30], rax

  ; save dir_path into file var
  mov   rsi, [rsp+0x8]
  mov   rdi, [rsp+0x30]

  ; add slash before dir path
  mov   rax, SLASH
  stosb

  call  strcpy
  cmp   rax, 0
  jl    .error

  mov   rdi, rax

  ; load null char
  mov   rax, NULL_CHAR
  stosb
  
  mov   rdi, [rsp+0x8]

  ; open directory
  mov   rsi, O_RDONLY | O_DIRECTORY
  mov   rdx, 0
  call  open_file

  cmp   rax, 0
  jl    .error

  ; get dirent struct
  mov   rdi, rax
  mov   rax, SYS_GETDENTS
  mov   rsi, [rsp+0x28]
  mov   rdx, DIR_ENTRIES_LEN
  syscall

  cmp   rax, 0
  jl    .error

  ; store number of bytes read
  mov   qword [rsp], rax

  ; save file_path_len
  mov   rdi, [rsp+0x30]
  call  strlen
  mov   qword [rsp+0x10], rax

  xor   r9, r9
  mov   rdi, [rsp+0x28]
  
.loop:
  ; d_reclen
  xor   r10, r10

  ; mov to d_reclen (offset 16)
  mov   r10w, word [rdi + 0x10]

  ; get d_type (offset d_reclen - 1)
  xor   rax, rax
  mov   al, byte [rdi + r10 - 1]

  ; check if it's a file (DT_REG = 0x8)
  cmp   rax, DT_REG
  jne   .skip_not_file

  ; save rdi pos
  mov   [rsp+0x8], rdi

  ; clear file path after dir
  mov   rdi, [rsp+0x30]
  mov   rax, qword [rsp+0x10]
  add   rdi, rax
  mov   rax, SLASH
  stosb
  call  strlen

  mov   rcx, rax
  xor   rax, rax
  rep   stosb

  mov   rdi, [rsp+0x30] 
  mov   rax, qword [rsp+0x10]
  add   rdi, rax
  inc   rdi

  mov   rsi, [rsp+0x8]

  ; go to file name (offset 18)
  add   rsi, 18

  call  strcpy
  cmp   rax, 0
  jl    .error

  mov   rdi, rax

  ; add null char at the end
  mov   rax, NULL_CHAR
  stosb

  mov   rdi, [rsp+0x30]
  call  get_content_type

  cmp   rax, 0
  jl    .next

  cmp   rax, CONTENT_APL
  je    .next

  mov   rdi, [rsp+0x40]
  lea   rsi, [GET]
  mov   rdx, [rsp+0x30] 
  mov   rcx, serve_dir
  xor   r8, r8
  call  add_route

.next:
  mov   rdi, [rsp+0x8]

  jmp   .skip_not_dir

.skip_not_file:
  cmp   rax, DT_DIR
  jne   .skip_not_dir

  ; make sure the dir is not '.'
  cmp   byte [rdi+18], DOT
  je    .skip_not_dir 

  ; save r9, r10 and rdi
  mov   qword [rsp+0x18], r9
  mov   qword [rsp+0x20], r10
  mov   [rsp+0x8], rdi

  ; clear file path after dir
  mov   rdi, [rsp+0x30]
  mov   rax, qword [rsp+0x10]
  add   rdi, rax
  mov   rax, SLASH
  stosb
  call  strlen

  mov   rcx, rax
  xor   rax, rax
  rep   stosb

  mov   rdi, [rsp+0x30] 
  mov   rax, qword [rsp+0x10]
  add   rdi, rax
  inc   rdi

  mov   rsi, [rsp+0x8]

  ; go to file name (offset 18)
  add   rsi, 18

  call  strcpy
  cmp   rax, 0
  jl    .error

  mov   rdi, rax

  ; add null char at the end
  mov   rax, NULL_CHAR
  stosb

  mov   rdi, [rsp+0x30]

  ; add one to remove slash at the beginning
  cmp   qword [rsp+0x38], TRUE
  jne   .no_recursion

  inc   rdi
  mov   rsi, rdi
  mov   rdi, [rsp+0x40]
  mov   rdx, TRUE
  xor   r8, r8
  call  add_dir_route

.no_recursion:
  mov   r9, qword [rsp+0x18]
  mov   r10, qword [rsp+0x20]
  mov   rdi, [rsp+0x8]

.skip_not_dir:
  add   r9, r10
  add   rdi, r10

  cmp   r9, qword [rsp]
  jge   .return
  jmp   .loop

.return:
  mov   rdi, [rsp+0x28]
  call  free

  mov   rdi, [rsp+0x30]
  call  free

  mov   rax, SUCCESS_CODE
  add   rsp, 0x48
  ret

.error:
  mov   rdi, [rsp+0x28]
  call  free

  mov   rdi, [rsp+0x30]
  call  free

  mov   rax, FAILURE_CODE
  add   rsp, 0x48
  ret

; checks if a route already exists
; @param  rdi: pointer to server struct
; @param  rsi: pointer to method
; @param  rdx: pointer to url
; @return rax: (bool) route exist
route_exist:
  sub   rsp, 0x20

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the server struct
  ; [rsp+0x8]   -> pointer to the method
  ; [rsp+0x10]  -> pointer to the url
  ; [rsp+0x18]  -> pointer to routes linked list

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  cmp   rdx, 0
  jle   .error

  ; load server routes
  mov   rdi, [rsp]
  call  get_server_routes
  cmp   rax, 0
  jl    .error
  je    .loop_end

  ; save route address
  mov   [rsp+0x18], rax

.loop:
  ; for each route
  ; check if the url is the same
  mov   rdi, [rsp+0x18]
  call  get_route_url

  cmp   rax, 0
  jl    .error

  mov   rsi, rax
  mov   rdi, [rsp+0x10]  
  call  strcmp

  cmp   rax, 0
  jl    .error
  je    .not_equal

  mov   rdi, [rsp+0x18]
  call  get_route_method

  cmp   rax, 0
  jl    .error

  mov   rsi, rax
  mov   rdi, [rsp+0x10]  
  call  strcmp  

  cmp   rax, 0
  jl    .error
  je    .equal

  ; go to next
.not_equal:
  mov   rdi, [rsp+0x18]
  mov   rsi, [rdi+ROUTE_OFF_NEXT]

  test  rsi, rsi
  jz    .loop_end

  mov   [rsp+0x18], rsi

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
  add   rsp, 0x20
  ret

; adds a route to the server
; @param  rdi: pointer to server struct
; @param  rsi: pointer to method
; @param  rdx: pointer to url
; @param  rcx: pointer to callback
; @param  r8: pointer to the group struct or 0 if root
; @return rax: pointer to route struct
add_route:
  sub   rsp, 0x48

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the server struct
  ; [rsp+0x8]   -> pointer to the method of the route
  ; [rsp+0x10]  -> pointer to the url of the route
  ; [rsp+0x18]  -> pointer to the callback function
  ; [rsp+0x20]  -> pointer to the group struct
  ; [rsp+0x28]  -> pointer to the malloc'd route struct
  ; [rsp+0x30]  -> size of the url
  ; [rsp+0x38]  -> pointer to the malloc'd url
  ; [rsp+0x40]  -> pointer to the concatenated prefix and url

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   [rsp+0x18], rcx
  mov   [rsp+0x20], r8
  mov   qword [rsp+0x28], 0
  mov   qword [rsp+0x38], 0
  mov   qword [rsp+0x40], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  cmp   rdx, 0
  jle   .error

  ; if group is not null, concatenate group prefix and route url
  mov   rdi, [rsp+0x20]
  test  rdi, rdi
  jz    .no_group

  ; get the group prefix
  call  get_group_prefix
  cmp   rax, 0
  jl    .error

  mov   rdi, rax 
  mov   rsi, [rsp+0x10]
  call  strcat
  cmp   rax, 0
  jl    .error
  
  mov   [rsp+0x40], rax
  mov   [rsp+0x10], rax

.no_group:
  ; make sure the route does not already exist
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x8]
  mov   rdx, [rsp+0x10]
  call  route_exist
  cmp   rax, 0
  jl    .error 

  cmp   rax, TRUE
  je    .error

  ; make sure method is valid
  mov   rdi, [rsp+0x8]
  call  is_valid_method
  cmp   rax, 0
  je    .error

  ; malloc route struct
  mov   rdi, ROUTE_STRUCT_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x28], rax 

  ; add method to struct
  mov   rdi, [rsp+0x8]
  mov   [rax+ROUTE_OFF_METHOD], rdi

  ; add callback
  mov   rdi, [rsp+0x18]
  mov   [rax+ROUTE_OFF_CALLBACK], rdi

  ; set next to 0
  mov   qword [rax+ROUTE_OFF_NEXT], 0

  ; set the group
  mov   rdi, [rsp+0x20]
  test  rdi, rdi
  jnz   .no_root_group

  mov   rdi, [rsp]
  call  get_server_root_group
  cmp   rax, 0
  jl    .error

  mov   rdi, rax

.no_root_group:
  mov   rax, [rsp+0x28]
  mov   qword [rax+ROUTE_OFF_GROUP], rdi

  ; get url length
  mov   rdi, [rsp+0x10]
  call  strlen
  cmp   rax, 0
  jl    .error

  ; make sure is less than MAX_LEN
  cmp   rax, URL_MAX_LEN
  jg    .error

  mov   [rsp+0x30], rax

  cmp   rax, 1
  je    .malloc_url

  ; check if the url ends with '/', if yes, trim it
  xor   rax, rax
  mov   rsi, [rsp+0x10]
  cmp   byte [rsi], SLASH
  jne   .error

  add   rsi, [rsp+0x30]
  dec   rsi
  lodsb

  cmp   rax, SLASH
  jne   .malloc_url

  dec   qword [rsp+0x30]

.malloc_url:
  ; malloc url
  mov   rdi, [rsp+0x30]
  inc   rdi       ; increase by one to have null char
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x38], rax

  ; copy url
  mov   rdi, rax
  mov   rsi, [rsp+0x10]
  mov   rcx, [rsp+0x30]
  rep   movsb

  xor   rax, rax
  stosb

  ; add to url struct
  mov   rdi, [rsp+0x38]
  mov   rax, [rsp+0x28]
  mov   [rax+ROUTE_OFF_URL], rdi

  ; add route to server routes
  mov   rdi, [rsp]
  mov   rsi, rax
  call  add_route_struct_to_server
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x28]

  jmp   .return

.error:
  mov   rdi, [rsp+0x40]
  test  rdi, rdi
  jz    .no_free_url

  call  free

.no_free_url:
  mov   rdi, [rsp+0x28]
  test  rdi, rdi
  jz    .no_free_struct

  call  free

.no_free_struct:
  mov   rdi, [rsp+0x38] 
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x48
  ret

; adds a structure of route to the list of routes available
; @param  rdi: pointer to server struct
; @param  rsi: pointer to route struct
; @return rax: return code
add_route_struct_to_server:
  sub   rsp, 0x10

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  ; get route offset to check if routes are init
  add   rdi, SERVER_OFF_ROUTES

  cmp   qword [rdi], 0
  je    .add

  ; load first route
  mov   rsi, [rdi]
  mov   rdi, rsi
  
.loop:
  ; rdi points to the address of current route
  ; go to next offset
  add   rdi, ROUTE_OFF_NEXT
  cmp   qword [rdi], 0
  je    .add
 
  ; rdi contains the address of the route
  mov   rsi, [rdi]
  mov   rdi, rsi

  jmp   .loop

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

; returns if the url is a wildcard (ends with '*')
; @param  rdi: pointer to the route
; @return rax: (bool) is a wildcard route
is_wildcard_route:
  sub   rsp, 0x8
  
  cmp   rdi, 0
  jl    .error

  mov   [rsp], rdi

  ; get string length
  mov   rdi, [rsp]
  call  strlen
  cmp   rax, 0
  jl    .error

  mov   rsi, [rsp]
  add   rsi, rax
  dec   rsi   ; remove one to get last char
  lodsb

  cmp   rax, ASTERISK
  jne   .not_wildcard

  mov   rax, TRUE
  jmp   .return

.not_wildcard:
  mov   rax, FALSE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret


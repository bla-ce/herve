section .data
  route_struct:
    route_method    dq 0  
    route_url       dq 0
    route_callback  dq 0
    route_next      dq 0
  route_struct_end:

  ROUTE_STRUCT_LEN equ route_struct_end - route_struct

  ROUTE_OFF_METHOD    equ 0x0
  ROUTE_OFF_URL       equ ROUTE_OFF_METHOD + 0x8
  ROUTE_OFF_CALLBACK  equ ROUTE_OFF_URL + 0x8
  ROUTE_OFF_NEXT      equ ROUTE_OFF_CALLBACK + 0x8

section .text
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

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  ; init to 0 -> this is for query parameters
  mov   qword [rsp+0x20], 0

  ; compare methods
  mov   rsi, [rsp]
  mov   rdi, [rsi+REQ_OFF_METHOD] 
  mov   rdx, [rsp+0x8]
  mov   rsi, [rdx+ROUTE_OFF_METHOD]
  call  cmpstr

  cmp   rax, 0
  jl    .error
  je    .not_the_same

  ; compare urls
  mov   rsi, [rsp]
  mov   rdi, [rsi+REQ_OFF_URL] 

  ; save url
  mov   [rsp+0x10], rdi

  ; check if there is any query parameters
  mov   rsi, QUESTION_MARK
  call  find_next_char

  cmp   rax, 0
  jl    .no_query

  ; save position
  mov   qword [rsp+0x18], rax

  ; malloc new url without string
  mov   rdi, rax
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  mov   rdi, rax
  mov   rsi, [rsp+0x10]
  mov   rcx, qword [rsp+0x18]
  rep   movsb

  mov   rdi, [rsp+0x20]
  mov   [rsp+0x10], rdi

.no_query:
  mov   rdi, [rsp+0x10]
  mov   rdx, [rsp+0x8]
  mov   rsi, [rdx+ROUTE_OFF_URL]
  call  cmpstr

  cmp   rax, 0
  jl    .error
  je    .not_the_same

  mov   rax, TRUE

  jmp   .return

.not_the_same:
  mov   rax, FALSE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
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

  ; add null char at the end
  mov   rax, NULL_CHAR
  stosb

  mov   rdi, [rsp+0x40]
  lea   rsi, [GET]
  mov   rdx, [rsp+0x30] 
  mov   rcx, serve_dir
  call  add_route

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

  ; add null char at the end
  mov   rax, NULL_CHAR
  stosb

  mov   rdi, [rsp+0x30]

  ; add one to remove slash at the beginning
  cmp   qword [rsp+0x38], 1
  jne   .no_recursion

  inc   rdi
  mov   rsi, rdi
  mov   rdi, [rsp+0x40]
  mov   rdx, 1
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

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  cmp   rdx, 0
  jle   .error

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx

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
  call  cmpstr

  cmp   rax, 0
  jl    .error
  je    .not_equal

  mov   rdi, [rsp+0x18]
  call  get_route_method

  cmp   rax, 0
  jl    .error

  mov   rsi, rax
  mov   rdi, [rsp+0x10]  
  call  cmpstr  

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
; @return rax: pointer to route struct
add_route:
  sub   rsp, 0x38

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  cmp   rdx, 0
  jle   .error

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   [rsp+0x18], rcx

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

  mov   [rsp+0x20], rax 

  ; add method to struct
  mov   rdi, [rsp+0x8]
  mov   [rax+ROUTE_OFF_METHOD], rdi

  ; add callback
  mov   rdi, [rsp+0x18]
  mov   [rax+ROUTE_OFF_CALLBACK], rdi

  ; set next to 0
  mov   qword [rax+ROUTE_OFF_NEXT], 0

  ; get url length
  mov   rdi, [rsp+0x10]
  call  strlen

  ; make sure is less than MAX_LEN
  cmp   rax, URL_MAX_LEN
  jg    .error

  mov   [rsp+0x28], rax

  ; malloc url
  inc   rax       ; increase by one to have null char
  mov   rdi, rax
  call  malloc

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x30], rax

  ; copy url
  mov   rdi, rax
  mov   rsi, [rsp+0x10]
  mov   rcx, [rsp+0x28]
  rep   movsb

  xor   rax, rax
  stosb

  ; add to url struct
  mov   rdi, [rsp+0x30]
  mov   rax, [rsp+0x20]
  mov   [rax+ROUTE_OFF_URL], rdi

  ; add route to server routes
  mov   rdi, [rsp]
  mov   rsi, rax
  call  add_route_struct_to_server

  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x20]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x38
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


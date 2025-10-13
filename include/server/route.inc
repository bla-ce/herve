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

  ; check if compared route is a dynamic route
  mov   rsi, [rsp+0x8]
  mov   rdi, [rsi+ROUTE_OFF_URL]
  call  is_dynamic_route
  cmp   rax, TRUE
  je    .handle_dynamic_route

  mov   rdi, [rsp+0x10]
  mov   rdx, [rsp+0x8]
  mov   rsi, [rdx+ROUTE_OFF_URL]
  call  strcmp
  cmp   rax, 0
  jl    .error
  je    .not_the_same

  jmp   .equal

.handle_dynamic_route:
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp]
  mov   rdx, [rsp+0x10]
  call  handle_dynamic_route
  cmp   rax, 0
  jl    .error
  je    .not_the_same

  jmp   .equal

.cmp_wildcard:
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x10]
  call  cmp_wildcard
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
; @return rax: code
add_dir_route:
  sub   rsp, 0x20
  
  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the server struct
  ; [rsp+0x8]   -> pointer to the dir path
  ; [rsp+0x10]  -> length of the dir path
  ; [rsp+0x18]  -> pointer to the route url

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error
  
  ; check if dir path ends with '/', if yes, trim it
  mov   rdi, [rsp+0x8]
  call  strlen
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  mov   rdi, [rsp+0x8]
  add   rdi, rax
  dec   rdi
  cmp   byte [rdi], SLASH
  jne   .no_trim

  mov   rax, NULL_CHAR
  stosb

.no_trim:
  ; malloc route url
  mov   rdi, FILE_PATH_LEN
  call  malloc
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  mov   rdi, [rsp+0x18]
  mov   rsi, [rsp+0x8]

  ; add slash before dir path
  mov   rax, SLASH
  stosb

  call  strcpy
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x18]
  add   rdi, [rsp+0x10]

  ; add '/' and '*'
  mov   rax, SLASH
  stosb

  mov   rax, ASTERISK
  stosb

  ; load null char
  mov   rax, NULL_CHAR
  stosb
  
  ; create wildcard route
  mov   rdi, [rsp]
  lea   rsi, [GET]
  mov   rdx, [rsp+0x18]
  mov   rcx, serve_dir
  xor   r8, r8
  call  add_route
  cmp   rax, 0
  jl    .error

  ; free route url
  mov   rdi, [rsp+0x18]
  call  free
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

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
  call  server_get_root_group
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

; returns if the url is a dynamic route
; @param  rdi: pointer to the route
; @return rax: (bool) is a dynamic route
is_dynamic_route:
  cmp   rdi, 0
  jl    .error

  mov   rsi, COLON
  call  find_next_char
  cmp   rax, 0
  jl    .not_dynamic
  je    .error

  mov   rax, TRUE
  jmp   .return

.not_dynamic:
  mov   rax, FALSE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
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

; compares the current route with a wildcard route
; @param  rdi: pointer to the wildcard route struct
; @param  rsi: pointer to the current url
; @return rax: same route (bool)
cmp_wildcard:
  sub   rsp, 0x10

  ; *** STACK USAGE *** ;
  ; [rsp]     -> pointer to the route struct
  ; [rsp+0x8] -> pointer to the current url

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  mov   rsi, [rsp]
  mov   rdi, [rsi+ROUTE_OFF_URL]
  mov   rsi, ASTERISK
  call  find_next_char
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   rdx, [rsp]
  mov   rsi, [rdx+ROUTE_OFF_URL]
  mov   rdx, rax
  call  strncmp
  jl    .error

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; compares the current route with a dynamic route and
; populates a hash table with the parameters and the values
; the hash table will be saved inside request struct
; @param  rdi: pointer to the dynamic route struct
; @param  rsi: pointer to the request struct
; @param  rdx: pointer to the request url
; @return rax: same route (bool)
handle_dynamic_route:
  sub   rsp, 0x48

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to the route struct
  ; [rsp+0x8]   -> pointer to the request struct
  ; [rsp+0x10]  -> pointer to the request url
  ; [rsp+0x18]  -> pointer to the url of the route struct
  ; [rsp+0x20]  -> number of parameter
  ; [rsp+0x28]  -> pointer to the hash table
  ; [rsp+0x30]  -> offset to next slash
  ; [rsp+0x38]  -> pointer to the key of the parameter
  ; [rsp+0x40]  -> pointer to the value of the parameter

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   qword [rsp+0x28], 0
  mov   qword [rsp+0x38], 0
  mov   qword [rsp+0x40], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  cmp   rdx, 0
  jle   .error

  mov   rsi, [rsp]
  mov   rdi, [rsi+ROUTE_OFF_URL]
  mov   [rsp+0x18], rdi

  ; count the number of parameter
  mov   rdi, [rsp+0x18]
  mov   rsi, COLON
  call  count_char
  cmp   rax, 0
  jle   .error

  mov   [rsp+0x20], rax

  ; create hash table
  mov   rdi, rax
  call  ht_create
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x28], rax

.loop:
  ; check if there is another parameter to get
  mov   rdi, [rsp+0x18]
  mov   rsi, COLON
  call  find_next_char
  cmp   rax, 0
  jge   .compare  ; if yes, compare until the parameter

  ; if not, get the remaining length to compare
  mov   rdi, [rsp+0x18]
  call  strlen
  cmp   rax, 0
  jl    .error

.compare:
  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp+0x18]
  mov   rcx, rax
  rep   cmpsb
  jne    .not_equal

  mov   [rsp+0x10], rdi
  mov   [rsp+0x18], rsi

  ; check if we are at the end (null char)
  xor   rax, rax
  add   al, byte [rdi]
  add   al, byte [rsi]
  test  rax, rax
  jz    .equal

  ; go after the parameter and compare the rest of the url
  mov   rdi, [rsp+0x18]
  mov   rsi, SLASH
  call  find_next_char
  cmp   rax, 0
  jg    .move_after_param

  mov   rdi, [rsp+0x10]
  mov   rsi, SLASH
  call  find_next_char
  cmp   rax, 0
  je    .loop_end
  jl    .equal

  jmp   .not_equal

.move_after_param:
  ; save offset
  mov   [rsp+0x30], rax

  ; get key
  mov   rdi, [rsp+0x18]
  inc   rdi         ; go after the '<'
  mov   rsi, rax
  dec   rsi         ; remove the '<'
  call  strndup
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x38], rax

  mov   rdi, [rsp+0x18]
  add   rdi, [rsp+0x30]
  mov   [rsp+0x18], rdi

  mov   rdi, [rsp+0x10]
  mov   rsi, SLASH
  call  find_next_char
  cmp   rax, 0
  jle   .not_equal

  ; save offset
  mov   [rsp+0x30], rax

  ; get value
  mov   rdi, [rsp+0x10]
  mov   rsi, rax
  call  strndup
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x40], rax

  mov   rdi, [rsp+0x10]
  add   rdi, [rsp+0x30]
  mov   [rsp+0x10], rdi

  ; insert to hash table
  mov   rdi, [rsp+0x28]
  mov   rsi, [rsp+0x38]
  mov   rdx, [rsp+0x40]
  call  ht_insert
  cmp   rax, 0
  jl    .error

  ; free key
  mov   rdi, [rsp+0x38]
  call  free
  cmp   rax, 0
  jl    .error

  mov   qword [rsp+0x38], 0

  ; free value
  mov   rdi, [rsp+0x40]
  call  free
  cmp   rax, 0
  jl    .error

  mov   qword [rsp+0x40], 0

  jmp   .loop

.loop_end:
  mov   rdi, [rsp+0x10]
  mov   rsi, [rsp+0x18]
  call  strcmp
  cmp   rax, TRUE
  jne   .not_equal

.equal:
  ; set dynamic parameters ht to request
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x28]
  call  set_request_dynamic_param
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x38]
  mov   rsi, [rsp+0x40]
  xor   rdx, rdx        ; we don't free the ht here
  call  free_kv_dyn_param

  mov   rax, TRUE

  jmp   .return

.not_equal:
  mov   rdi, [rsp+0x38]
  mov   rsi, [rsp+0x40]
  mov   rdx, [rsp+0x28]
  call  free_kv_dyn_param

  mov   rax, FALSE
  jmp   .return

.error:
  mov   rdi, [rsp+0x38]
  mov   rsi, [rsp+0x40]
  mov   rdx, [rsp+0x28]
  call  free_kv_dyn_param

  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x48
  ret

; frees the key, the value and the hash table if previously mallocd
; @param  rdi: pointer to the key
; @param  rsi: pointer to the value
; @param  rdx: pointer to the hash table
; @return rax: return code
free_kv_dyn_param:
  sub   rsp, 0x18

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx

  mov   rdi, [rsp]
  test  rdi, rdi
  jz    .no_key_free

  call  free

.no_key_free:
  mov   rdi, [rsp+0x8]
  test  rdi, rdi
  jz    .no_value_free

  call  free

.no_value_free:
  mov   rdi, [rsp+0x10]
  test  rdi, rdi
  jz    .return

  call  ht_free

.return:
  add   rsp, 0x18
  ret

; returns dynamic parameter
; @param  rdi: pointer to request struct
; @param  rsi: key
; @return rax: value
get_dynamic_param:
  sub   rsp, 0x10

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; get query hash table
  mov   rdi, [rsp]
  call  get_request_dynamic_param
  cmp   rax, 0
  jle   .error

  mov   rdi, rax
  mov   rsi, [rsp+0x8]
  call  ht_get
  cmp   rax, 0
  jl    .error

  jmp   .return
  
.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret


section .bss
route_t:
  .method             resq 1
  .url                resq 1
  .callback           resq 1
  .group              resq 1
  .active             resq 1
  .arg1               resq 1
  .next_server_route  resq 1  ; route linked list of the server
  .next_group_route   resq 1  ; route linked list of the group
route_t_end:

section .data
  ROUTE_STRUCT_LEN equ route_t_end - route_t

  ROUTE_OFF_METHOD      equ 0x0
  ROUTE_OFF_URL         equ ROUTE_OFF_METHOD + 0x8
  ROUTE_OFF_CALLBACK    equ ROUTE_OFF_URL + 0x8
  ROUTE_OFF_GROUP       equ ROUTE_OFF_CALLBACK + 0x8
  ROUTE_OFF_ACTIVE      equ ROUTE_OFF_GROUP + 0x8
  ROUTE_OFF_ARG1        equ ROUTE_OFF_ACTIVE + 0x8
  ROUTE_OFF_SERVER_NEXT equ ROUTE_OFF_ARG1 + 0x8
  ROUTE_OFF_GRP_NEXT    equ ROUTE_OFF_SERVER_NEXT + 0x8

  log_pre_routes  db SPACE, SPACE, NULL_CHAR

  http_model_route_root db "/", NULL_CHAR
  http_model_route_id   db "/:id", NULL_CHAR

  health_check_url  db "/health", NULL_CHAR

section .text
; logs the route "METHOD  URL"
; @param  rdi: pointer to the route
; @return rax: return code
route_log:
  sub   rsp, 0x8

  ; STACK USAGE
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
route_get_group:
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
route_get_url:
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
route_get_method:
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
route_get_callback:
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
route_match_request:
  sub   rsp, 0x30

  cmp   rdi, 0
  jl    .error

  cmp   rsi, 0
  jl    .error

  ; STACK USAGE
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
  call  route_is_wildcard
  cmp   rax, TRUE
  je    .match_wildcard

  ; check if compared route is a dynamic route
  mov   rsi, [rsp+0x8]
  mov   rdi, [rsi+ROUTE_OFF_URL]
  call  route_is_dynamic
  cmp   rax, TRUE
  je    .match_dynamic

  mov   rdi, [rsp+0x10]
  mov   rdx, [rsp+0x8]
  mov   rsi, [rdx+ROUTE_OFF_URL]
  call  strcmp
  cmp   rax, 0
  jl    .error
  je    .not_the_same

  jmp   .equal

.match_dynamic:
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp]
  mov   rdx, [rsp+0x10]
  call  route_match_dynamic
  cmp   rax, 0
  jl    .error
  je    .not_the_same

  jmp   .equal

.match_wildcard:
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x10]
  call  route_match_wildcard
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
; @param  rsi: pointer to the root url
; @return rax: return code
route_add_directory:
  sub   rsp, 0x20

  ; STACK USAGE
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
  mov		r8, NO_ARG
  call  route_add
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
; @return rax: (bool) route exists
route_exists:
  sub   rsp, 0x20

  ; STACK USAGE
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
  call  route_get_url
  cmp   rax, 0
  jl    .error

  mov   rsi, rax
  mov   rdi, [rsp+0x10]
  call  strcmp
  cmp   rax, 0
  jl    .error
  cmp   rax, TRUE
  jne   .not_equal

  mov   rdi, [rsp+0x18]
  call  route_get_method
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  strcmp
  cmp   rax, TRUE
  je    .equal

  ; go to next
.not_equal:
  mov   rdi, [rsp+0x18]
  mov   rsi, [rdi+ROUTE_OFF_SERVER_NEXT]

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
; @param  r9: arg 1 for the callback function
; @return rax: pointer to route struct
route_add:
  sub   rsp, 0x50

  ; STACK USAGE
  ; [rsp]       -> pointer to the server struct
  ; [rsp+0x8]   -> pointer to the method of the route
  ; [rsp+0x10]  -> pointer to the url of the route
  ; [rsp+0x18]  -> pointer to the callback function
  ; [rsp+0x20]  -> pointer to the group struct
  ; [rsp+0x28]  -> pointer to the malloc'd route struct
  ; [rsp+0x30]  -> size of the url
  ; [rsp+0x38]  -> pointer to the malloc'd url
  ; [rsp+0x40]  -> pointer to the concatenated prefix and url
  ; [rsp+0x48]  -> argument for callback function

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   [rsp+0x18], rcx
  mov   [rsp+0x20], r8
  mov   qword [rsp+0x28], 0
  mov   qword [rsp+0x38], 0
  mov   qword [rsp+0x40], 0
  mov   [rsp+0x48], r9

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
  call  group_get_prefix
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
  call  route_exists
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

  mov   qword [rax+ROUTE_OFF_SERVER_NEXT], 0
  mov   qword [rax+ROUTE_OFF_GRP_NEXT], 0

  mov   qword [rax+ROUTE_OFF_ACTIVE], TRUE

  ; set the group
  mov   rdi, [rsp+0x20]
  test  rdi, rdi
  jnz   .no_root_group

  mov   rdi, [rsp]
  call  server_get_root_group
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  mov   [rsp+0x20], rax

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

  mov   rdi, [rsp+0x48]
  mov   [rax+ROUTE_OFF_ARG1], rdi

  ; add route to group routes
  mov   rdi, [rsp+0x20]
  mov   rsi, [rsp+0x28]
  call  route_link_to_group
  cmp   rax, 0
  jl    .error

  ; add route to server routes
  mov   rdi, [rsp]
  mov   rsi, [rsp+0x28]
  call  route_link_to_server
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
  add   rsp, 0x50
  ret

; links route struct to the server's route linked list
; @param  rdi: pointer to server struct
; @param  rsi: pointer to route struct
; @return rax: return code
route_link_to_server:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; get route offset to check if routes are init
  add   rdi, SERVER_OFF_ROUTES
  mov   rdx, ROUTE_OFF_SERVER_NEXT
  call  linked_list_add_entry
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; links route struct to the group's route linked list
; @param  rdi: pointer to group struct
; @param  rsi: pointer to route struct
; @return rax: return code
route_link_to_group:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; get route offset to check if routes are init
  add   rdi, GRP_OFF_ROUTES

  mov   rdx, ROUTE_OFF_GRP_NEXT
  call  linked_list_add_entry
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns if the url is a dynamic route
; @param  rdi: pointer to the route
; @return rax: (bool) is a dynamic route
route_is_dynamic:
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
route_is_wildcard:
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
route_match_wildcard:
  sub   rsp, 0x10

  ; STACK USAGE
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
route_match_dynamic:
  sub   rsp, 0x40

  ; STACK USAGE
  ; [rsp]       -> pointer to the route struct
  ; [rsp+0x8]   -> pointer to the request struct
  ; [rsp+0x10]  -> pointer to the request url
  ; [rsp+0x18]  -> dynamic pointer to the request url
  ; [rsp+0x20]  -> pointer to the url of the route struct
  ; [rsp+0x28]  -> dynamic pointer to the url of the route struct
  ; [rsp+0x30]  -> offset of the next parameter
  ; [rsp+0x38]  -> hash table to store the parameters

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   [rsp+0x18], rdx
  mov   [rsp+0x38], rdx

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  cmp   rdx, 0
  jle   .error

  mov   rdi, [rsp]
  call  route_get_url
  cmp   rax, 0
  jl   .error

  mov   [rsp+0x20], rax
  mov   [rsp+0x28], rax

  ; get number of parameters to create hash table
  mov   rdi, [rsp+0x20]
  mov   rsi, COLON
  call  count_char
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  ht_create
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x38], rax

  ; check that the routes are equal
  mov   rdi, [rsp+0x18]
  mov   rsi, [rsp+0x28]

.loop:
  ; get the offset of the next parameter
  mov   rdi, [rsp+0x28]
  mov   rsi, COLON
  call  find_next_char
  cmp   rax, 0
  jl    .get_remaining_length_to_compare

  mov   rdx, rax
  mov   [rsp+0x30], rax

  jmp   .compare

.get_remaining_length_to_compare:
  mov   rdi, [rsp+0x28]
  call  strlen
  cmp   rax, 0
  jl    .error
  je    .equal

  mov   rdx, rax


  mov   rdi, [rsp+0x18]
  mov   rsi, [rsp+0x28]
  ; rdx is already populated
  call  strncmp
  cmp   rax, 0
  jl    .error
  cmp   rax, TRUE
  je    .equal

.compare:
  mov   rdi, [rsp+0x18]
  mov   rsi, [rsp+0x28]
  ; rdx is already populated
  call  strncmp
  cmp   rax, 0
  jl    .error

  cmp   rax, TRUE
  jne   .not_equal

  ; go just before the parameter
  mov   rdi, [rsp+0x18]
  add   rdi, qword [rsp+0x30]
  mov   [rsp+0x18], rdi

  mov   rdi, [rsp+0x28]
  add   rdi, qword [rsp+0x30]
  mov   [rsp+0x28], rdi

  ; save parameter
  mov   rdi, [rsp+0x38]
  mov   rsi, [rsp+0x28]
  mov   rdx, [rsp+0x18]
  call  route_save_param
  cmp   rax, 0
  jl    .error

  ; go after the parameter
  mov   rdi, [rsp+0x18]
  mov   rsi, SLASH
  call  find_next_char
  cmp   rax, 0
  jl    .end_request_url

  mov   rdi, [rsp+0x18]
  add   rdi, rax
  mov   [rsp+0x18], rdi

  mov   rdi, [rsp+0x28]
  mov   rsi, SLASH
  call  find_next_char
  cmp   rax, 0
  jl    .not_equal

  mov   rdi, [rsp+0x28]
  add   rdi, rax
  mov   [rsp+0x28], rdi

  jmp   .loop

.end_request_url:
  ; check if it's the end of the route too
  mov   rdi, [rsp+0x28]
  mov   rsi, SLASH
  call  find_next_char
  cmp   rax, 0
  jl    .equal

  jmp   .not_equal

.equal:
  ; set dynamic parameters ht to request
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp+0x38]
  call  set_request_dynamic_param
  cmp   rax, 0
  jl    .error

  mov   rax, TRUE
  jmp   .return

.not_equal:
  mov   rdi, [rsp+0x38]
  call  ht_free
  cmp   rax, 0
  jl    .error

  mov   rax, FALSE
  jmp   .return

.error:
  mov   rdi, [rsp+0x38]
  test  rdi, rdi
  jz    .no_free

  call  ht_free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x40
  ret

; saves the current parameter inside the hash table
; @param  rdi: pointer to the hash table
; @param  rsi: pointer to the route url
; @param  rdx: pointer to the request url
; @return rax: return code
route_save_param:
  sub   rsp, 0x28

  ; STACK USAGE
  ; [rsp]       -> pointer to the hash table
  ; [rsp+0x8]   -> pointer to the route url
  ; [rsp+0x10]  -> pointer to the request url
  ; [rsp+0x18]  -> pointer to the dup key
  ; [rsp+0x20]  -> pointer to the dup value

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi
  mov   [rsp+0x10], rdx
  mov   qword [rsp+0x18], 0
  mov   qword [rsp+0x20], 0

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  cmp   rdx, 0
  jle   .error

  ; get parameter name
  mov   rdi, [rsp+0x8]
  inc   rdi ; go past ':'
  mov   rsi, SLASH
  call  find_next_char
  cmp   rax, 0
  jl    .get_key_full_length

  mov   rsi, rax
  jmp   .dup_key

.get_key_full_length:
  mov   rdi, [rsp+0x8]
  inc   rdi ; go past ':'
  call  strlen
  cmp   rax, 0
  jl   .error

  mov   rsi, rax

.dup_key:
  mov   rdi, [rsp+0x8]
  inc   rdi
  ; rsi is already populated
  call  strndup
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x18], rax

  ; get parameter value
  mov   rdi, [rsp+0x10]
  mov   rsi, SLASH
  call  find_next_char
  cmp   rax, 0
  jl    .get_value_full_length

  mov   rsi, rax
  jmp   .dup_value

.get_value_full_length:
  mov   rdi, [rsp+0x10]
  call  strlen
  cmp   rax, 0
  jl   .error

  mov   rsi, rax

.dup_value:
  mov   rdi, [rsp+0x10]
  ; rsi is already populated
  call  strndup
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x20], rax

  mov   rdi, [rsp]
  mov   rsi, [rsp+0x18]
  mov   rdx, [rsp+0x20]
  call  ht_insert
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x18]
  mov   rsi, [rsp+0x20]
  mov		rdx, NO_ARG
  call  route_free_dyn_param
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rdi, [rsp+0x18]
  mov   rsi, [rsp+0x20]
  mov		rdx, NO_ARG
  call  route_free_dyn_param

  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x28
  ret

; frees the key, the value and the hash table if previously mallocd
; @param  rdi: pointer to the key
; @param  rsi: pointer to the value
; @param  rdx: pointer to the hash table
; @return rax: return code
route_free_dyn_param:
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
route_get_dynamic_param:
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
  jle   .error

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

; removes the route from the server and frees the struct
; @param  rdi: pointer to the server
; @param  rsi: pointer to the route
; @return rax: return code
route_free:
  sub   rsp, 0x10

  ; STACK USAGE
  ; [rsp]     -> pointer to the server
  ; [rsp+0x8] -> pointer to the route

  mov   [rsp], rdi
  mov   [rsp+0x8], rsi

  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jle   .error

  ; remove route from ll
  mov   rdi, [rsp]
  add   rdi, SERVER_OFF_ROUTES
  mov   rsi, [rsp+0x8]
  mov   rdx, ROUTE_OFF_SERVER_NEXT
  call  linked_list_remove_entry
  cmp   rax, 0
  jl    .error

  ; free url
  mov   rdi, [rsp+0x8]
  call  route_get_url
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  free
  cmp   rax, 0
  jl    .error

  ; free route
  mov   rdi, [rsp+0x8]
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

; sets the active property to false
; @param  rdi: pointer to the route struct
; @return rax: return code
route_deactivate:
  cmp   rdi, 0
  jle   .error

  mov   qword [rdi+ROUTE_OFF_ACTIVE], FALSE

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the active property to true
; @param  rdi: pointer to the route struct
; @return rax: return code
route_activate:
  cmp   rdi, 0
  jle   .error

  mov   qword [rdi+ROUTE_OFF_ACTIVE], TRUE

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the url of the route
; @param  rdi: pointer to the route
; @param  rsi: url of the route
; @return rax: return code
route_set_url:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   [rdi+ROUTE_OFF_URL], rsi

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the next pointer in the server ll
; @param  rdi: pointer to the route struct
; @return rax: next of the route
route_get_server_next:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+ROUTE_OFF_SERVER_NEXT]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the route next pointer in the server ll
; @param  rdi: pointer to the route
; @param  rsi: next of the route
; @return rax: return code
route_set_server_next:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   [rdi+ROUTE_OFF_SERVER_NEXT], rsi

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; returns the next pointer in the group ll
; @param  rdi: pointer to the route struct
; @return rax: next of the route
route_get_group_next:
  cmp   rdi, 0
  jle   .error

  mov   rax, [rdi+ROUTE_OFF_GRP_NEXT]

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

; sets the route next pointer in the group ll
; @param  rdi: pointer to the route
; @param  rsi: next of the route
; @return rax: return code
route_set_group_next:
  cmp   rdi, 0
  jle   .error

  cmp   rsi, 0
  jl    .error

  mov   [rdi+ROUTE_OFF_GRP_NEXT], rsi

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  ret

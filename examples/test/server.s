global  _start

%include "herve.inc"

section .text
middleware:
  sub   rsp, 0x8

  mov   [rsp], rdi

  mov   rdi, rsi
  call  println

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

test_wildcard:
  mov   rsi, OK
  lea   rdx, [wildcard_msg]
  call  send_string
  ret

test_redirect:
  mov   rsi, FOUND
  lea   rdx, [template_url]
  call  redirect

  ret

test_post:
  sub   rsp, 0x10
  
  cmp   rdi, 0
  jle   .error

  mov   [rsp], rdi

  mov   rdi, [rsp]
  call  get_ctx_request

  mov   rdi, rax
  call  get_request_body

  ; parse form
  mov   rdi, rax
  call  parse_query_param
  cmp   rax, 0
  jl    .error
  
  mov   [rsp+0x8], rax

  mov   rdi, [rsp+0x8]
  lea   rsi, [form_fname]
  call  ht_get

  mov   rdi, [rsp]
  mov   rsi, OK
  mov   rdx, rax
  call  send_string

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

test_basic_auth:
  sub   rsp, 0x10

  cmp   rdi, 0
  jle   .error

  mov   [rsp], rdi
  mov   qword [rsp+0x8], 0

  ; get headers
  mov   rdi, [rsp]
  call  get_ctx_request
  cmp   rax, 0
  jl    .error

  ; get authorization header
  mov   rdi, rax
  call  get_request_headers
  cmp   rax, 0
  jl    .error

  ; decode auth
  mov   rdi, rax
  lea   rsi, [AUTHORIZATION_HEADER]
  call  get_header_value
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  read_basic_auth
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; compare with expected string
  mov   rdi, rax
  lea   rsi, [expected_auth]
  call  strcmp
  cmp   rax, FALSE
  je    .error

  mov   rdi, [rsp]
  mov   rsi, OK
  call  send_no_content

  ; free decoded basic auth
  mov   rdi, [rsp+0x8]
  call  free

  mov   rax, SUCCESS_CODE
  jmp   .return

.error:
  mov   rdi, [rsp]
  mov   rsi, UNAUTHORIZED
  call  send_no_content

  mov   rax, [rsp+0x8]
  test  rax, rax
  jz    .no_free

  mov   rdi, rax
  call  free

.no_free:
  mov   rax, FAILURE_CODE
  
.return:
  add   rsp, 0x10
  ret

test_template:
  sub   rsp, 0x18
  
  mov   [rsp], rdi

  mov   rdi, 4
  call  ht_create

  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, rax
  lea   rsi, [title_key]
  lea   rdx, [title]
  call  ht_insert

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  lea   rsi, [header_key]
  lea   rdx, [header]
  call  ht_insert

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  lea   rsi, [content_key]
  lea   rdx, [content]
  call  ht_insert

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  lea   rsi, [footer_key]
  lea   rdx, [footer]
  call  ht_insert

  cmp   rax, 0
  jl    .error

  lea   rdi, [template_path]
  mov   rsi, [rsp+0x8]
  call  parse_template

  cmp   rax, 0
  jl    .error

  ; save boeuf buffer
  mov   [rsp+0x10], rax

  mov   rdi, [rsp] 
  mov   rsi, OK
  mov   rdx, rax
  call  send_HTML

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x10]
  call  boeuf_free
  cmp   rax, 0
  jl    .error

  ; free hash table
  mov   rdi, [rsp+0x8]
  call  ht_free
  cmp   rax, 0
  jl    .error

  mov   rax, SUCCESS_CODE

  jmp   .return

.error:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x18
  ret

test_no_content:
  sub   rsp, 0x8

  mov   [rsp], rdi

  mov   rdi, [rsp]
  call  get_ctx_request

  cmp   rax, 0
  jl    .error

  ; get query
  mov   rdi, rax
  lea   rsi, [name_query]
  call  get_query_param

  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  println

  mov   rdi, [rsp]
  mov   rsi, NO_CONTENT
  call  send_no_content

  jmp   .return

.error:
  mov   rdi, [rsp]
  mov   rsi, BAD_REQUEST
  lea   rdx, [error_no_query]
  call  send_string

  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

test_string:
  sub   rsp, 0x10

  mov   [rsp], rdi
  mov   qword [rsp+0x8], 0

  ; add cookie
  lea   rdi, [name]
  lea   rsi, [value]
  mov   rdx, 20
  mov   rcx, 1
  mov   r8, 1
  lea   r9, [localhost]
  call  create_cookie
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, [rsp]
  mov   rsi, rax
  call  set_cookie
  cmp   rax, 0
  jl    .error

  ; test print request headers
  mov   rdi, [rsp]
  call  get_ctx_request

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, OK  
  lea   rdx, [ok_msg]
  call  send_string
  cmp   rax, 0
  jl    .error

  ; free cookie struct
  mov   rdi, [rsp+0x8]
  call  free

  jmp   .return

.error: 
  mov   rdi, [rsp+0x8]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x10
  ret

test_static:
  lea   rsi, [index_path]
  call  send_static_file
  ret

_start:
  sub   rsp, 0x10

  ; *** STACK USAGE *** ;
  ; [rsp]       -> pointer to cli args
  ; [rsp+0x8]   -> pointer to server struct
  ; [rsp+0x10]  -> argv
  ; [rsp+0x18]  -> pointer to argc

  mov   rdi, [rsp+0x10]
  cmp   rdi, 2
  jl    .empty_cli

  lea   rsi, [rsp+0x18]
  call  parse_cli
  cmp   rax, 0
  jl    .error

  mov   [rsp], rax

  ; parse port
  mov   rsi, rax
  mov   rdi, [rsi+0x8] ; get argv[1]
  call  stoi
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  jmp   .server_init

.empty_cli:
  mov   rdi, 0

.server_init:
  call  server_init
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  mov   rdi, rax
  call  get_server_sockfd

  cmp   rax, 0
  jl    .error

  mov   qword [sockfd], rax

  mov   rdi, [rsp+0x8]
  lea   rsi, [GET]
  lea   rdx, [index_url]
  mov   rcx, test_static
  call  add_route

  cmp   rax, 0
  jl    .error

  ; add no content route
  mov   rdi, [rsp+0x8]
  lea   rsi, [GET]
  lea   rdx, [root_url]
  mov   rcx, test_no_content
  call  add_route

  cmp   rax, 0
  jl    .error

  ; add health route
  mov   rdi, [rsp+0x8]
  lea   rsi, [GET]
  lea   rdx, [health_url]
  mov   rcx, test_string
  call  add_route

  cmp   rax, 0
  jl    .error

  ; add template route
  mov   rdi, [rsp+0x8]
  lea   rsi, [GET]
  lea   rdx, [template_url]
  mov   rcx, test_template
  call  add_route

  cmp   rax, 0
  jl    .error

  ; add post route
  mov   rdi, [rsp+0x8]
  lea   rsi, [POST]
  lea   rdx, [post_url]
  mov   rcx, test_post
  call  add_route

  cmp   rax, 0
  jl    .error

  ; add basic auth route
  mov   rdi, [rsp+0x8]
  lea   rsi, [GET]
  lea   rdx, [basic_url]
  mov   rcx, test_basic_auth
  call  add_route

  cmp   rax, 0
  jl    .error

  ; add redirect route
  mov   rdi, [rsp+0x8]
  lea   rsi, [GET]
  lea   rdx, [redirect_url]
  mov   rcx, test_redirect
  call  add_route

  cmp   rax, 0
  jl    .error

  ; add wildcard route
  mov   rdi, [rsp+0x8]
  lea   rsi, [GET]
  lea   rdx, [wildcard_url]
  mov   rcx, test_wildcard
  call  add_route
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  lea   rsi, [dir_path]
  mov   rdx, TRUE
  call  add_dir_route
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, middleware
  lea   rdx, [hello]
  call  add_middleware

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  mov   rsi, middleware
  lea   rdx, [hello2]
  call  add_middleware

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp+0x8]
  call  run_server
  
  add   rsp, 0x10

  mov   rax, SYS_EXIT
  mov   rdi, SUCCESS_CODE
  syscall

.error:
  add   rsp, 0x10

  mov   rax, SYS_EXIT
  mov   rdi, FAILURE_CODE
  syscall

section .bss

section .data
  sockfd  dq 0

  root_url      db "/", NULL_CHAR
  health_url    db "/health", NULL_CHAR
  index_url     db "/index", NULL_CHAR
  template_url  db "/template", NULL_CHAR
  redirect_url  db "/redirect", NULL_CHAR
  basic_url     db "/basic-auth/", NULL_CHAR
  post_url      db "/post", NULL_CHAR
  wildcard_url  db "/wild/*", NULL_CHAR

  index_path    db "examples/test/views/index.html", NULL_CHAR
  template_path db "examples/test/views/template.apl", NULL_CHAR
  dir_path      db "examples/test/views", NULL_CHAR

  name_query  db "name", NULL_CHAR
  form_fname  db "fname", NULL_CHAR
  hello       db "Hello", NULL_CHAR
  hello2      db "Hello2", NULL_CHAR

  ok_msg          db "ok", NULL_CHAR
  middleware_msg  db "Hello, World!", NULL_CHAR
  wildcard_msg    db "Where are you coming from?", NULL_CHAR

  error_no_query  db "failed to get query parameter", NULL_CHAR

  error db "ERROR", NULL_CHAR

  title_key   db "Title", NULL_CHAR
  header_key  db "Header", NULL_CHAR
  content_key db "Content", NULL_CHAR
  footer_key  db "Footer", NULL_CHAR

  title   db "GOAT | HEHE", NULL_CHAR
  header  db "Be careful, you're walking on a place of divinity", NULL_CHAR
  content db "This page uses a template engine written in assembly", NULL_CHAR
  footer  db "Still here?", NULL_CHAR

  expected_auth db "test:password", NULL_CHAR

  custom_delimiter db " -> ", NULL_CHAR

  name      db "name", NULL_CHAR
  value     db "value", NULL_CHAR
  localhost db "localhost", NULL_CHAR


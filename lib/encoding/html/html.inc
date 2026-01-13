%include "html_errors.inc"

section .data

char_to_esc db DOUBLE_QUOTES,
            db SINGLE_QUOTE,
            db AMPERSAND,
            db LESS_THAN,
            db GREATER_THAN

double_quote_esc  db "&quot;", NULL_CHAR
single_quote_esc  db "&apos;", NULL_CHAR
ampersand_esc     db "&amp;", NULL_CHAR
less_than_esc     db "&lt;", NULL_CHAR
greater_than_esc  db "&gt;", NULL_CHAR

section .text

; returns a NEW allocated string encoded for HTML
; user is responsible of freeing the string
; @param  rdi: pointer to the string
; @return rax: pointer to encoded string
escape_html:
  sub   rsp, 0x28

  ; *** STACK USAGE  *** ;
  ; [rsp]       -> pointer to the string
  ; [rsp+0x8]   -> pointer to the boeuf buffer
  ; [rsp+0x10]  -> pointer to the final buffer
  ; [rsp+0x18]  -> position of the cursor
  ; [rsp+0x20]  -> position of the last escaped character

  mov   [rsp], rdi
  mov   qword [rsp+0x8], 0
  mov   qword [rsp+0x10], 0
  mov   qword [rsp+0x18], 0
  mov   qword [rsp+0x20], 0

  mov   qword [html_errno], HTML_INVALID_ARG

  cmp   rdi, 0
  jle   .error

.loop:
  mov   rsi, [rsp]
  mov   rax, qword [rsp+0x18]

  xor   rdi, rdi
  mov   dil, byte [rsi+rax]
  call  char_needs_html_escaping
  cmp   rax, FALSE
  je    .no_escaping

  cmp   qword [rsp+0x18], 0
  je    .copy_esc_char

  ; copy what does not need to be escaped into the boeuf buffer
  mov   rax, [rsp+0x18] ; current position
  mov   rbx, [rsp+0x20] ; position of the last char copied

  sub   rax, rbx    ; get number of bytes to copy

  cmp   rax, 0
  je    .copy_esc_char

  ; check if we need to create or append
  cmp   qword [rsp+0x8], 0
  jne   .append

  mov   rdi, [rsp]
  add   rdi, [rsp+0x20]
  mov   rsi, rax
  call  boeuf_ncreate
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  jmp   .copy_esc_char

.append:
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp]
  add   rsi, [rsp+0x20]
  mov   rdx, rax
  call  boeuf_nappend
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

.copy_esc_char:
  ; get escaped character
  mov   rsi, [rsp]
  add   rsi, [rsp+0x18]

  xor   rdi, rdi
  mov   dil, byte [rsi]
  call  get_html_esc_char
  cmp   rax, 0
  jl    .error

  cmp   qword [rsp+0x8], 0
  jne   .append_esc

  mov   rdi, rax
  call  boeuf_create
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; update position of the last char copied
  mov   rax, [rsp+0x18]
  inc   rax
  mov   [rsp+0x20], rax

  jmp   .no_escaping

.append_esc:
  mov   rdi, [rsp+0x8]
  mov   rsi, rax
  call  boeuf_append
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  ; update position of the last char copied
  mov   rax, [rsp+0x18]
  inc   rax
  mov   [rsp+0x20], rax

.no_escaping:
  inc   qword [rsp+0x18]

  mov   rdi, [rsp]
  add   rdi, qword [rsp+0x18]
  cmp   byte [rdi], NULL_CHAR
  jne   .loop

.loop_end:
  mov   rax, [rsp+0x18] ; current position
  mov   rbx, [rsp+0x20] ; position of the last char copied

  mov   rdi, [rsp]
  add   rdi, rbx
  cmp   byte [rdi], NULL_CHAR
  je    .copy_boeuf

  sub   rax, rbx    ; get number of bytes to copy

  ; check if we need to create or append
  cmp   qword [rsp+0x8], 0
  jne   .append_end

  mov   rdi, [rsp]
  add   rdi, [rsp+0x20]
  mov   rsi, rax
  call  boeuf_ncreate
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

  jmp   .copy_boeuf

.append_end:
  mov   rdi, [rsp+0x8]
  mov   rsi, [rsp]
  add   rsi, [rsp+0x20]
  mov   rdx, rax
  call  boeuf_nappend
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x8], rax

.copy_boeuf:
  ; copy the boeuf buffer into a string
  mov   rdi, [rsp+0x8]
  call  strdup
  cmp   rax, 0
  jl    .error

  mov   [rsp+0x10], rax

  mov   rdi, [rsp+0x8]
  call  boeuf_free
  cmp   rax, 0
  jl    .error

  mov   rax, [rsp+0x10]

  mov   qword [html_errno], HTML_NO_ERR

  jmp   .return

.error:
  mov   rdi, [rsp+0x8]
  test  rdi, rdi
  jz    .free_string

  call  boeuf_free

.free_string:
  mov   rdi, [rsp+0x10]
  test  rdi, rdi
  jz    .no_free

  call  free

.no_free:
  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x28
  ret

; checks wether a given character should be escaped
; @param  rdi: character
; @return rax: bool
char_needs_html_escaping:
  cmp   dil, AMPERSAND
  je    .true

  cmp   dil, DOUBLE_QUOTES
  je    .true

  cmp   dil, SINGLE_QUOTE
  je    .true

  cmp   dil, GREATER_THAN
  je    .true

  cmp   dil, LESS_THAN
  je    .true

.false:
  mov   rax, FALSE
  jmp   .return

.true:
  mov   rax, TRUE
  jmp   .return

.return:
  ret

; returns the html escaped character
; @param  rdi: character
; @return rax: pointer to the escaped character
get_html_esc_char:
  cmp   dil, AMPERSAND
  je    .ampersand

  cmp   dil, DOUBLE_QUOTES
  je    .double_quotes

  cmp   dil, SINGLE_QUOTE
  je    .single_quote

  cmp   dil, GREATER_THAN
  je    .greater_than

  cmp   dil, LESS_THAN
  je    .less_than

  mov   qword [html_errno], HTML_INVALID_CHAR

  ; invalid character
  jmp   .error

.ampersand:
  mov   rax, ampersand_esc
  jmp   .return

.double_quotes:
  mov   rax, double_quote_esc
  jmp   .return

.single_quote:
  mov   rax, single_quote_esc
  jmp   .return

.greater_than:
  mov   rax, greater_than_esc
  jmp   .return

.less_than:
  mov   rax, less_than_esc
  jmp   .return

.error:
  mov   rax, FAILURE_CODE
  ret

.return:
  mov   qword [html_errno], HTML_NO_ERR
  ret

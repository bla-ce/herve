section .data
STATUS_CODE_LENGTH equ 3

; Status Code Constants
CONTINUE                        equ 100
SWITCHING_PROTOCOLS             equ 101
PROCESSING                      equ 102
EARLY_HINTS                     equ 103

OK                              equ 200
CREATED                         equ 201
ACCEPTED                        equ 202
NON_AUTHORITATIVE_INFORMATION   equ 203
NO_CONTENT                      equ 204
RESET_CONTENT                   equ 205
PARTIAL_CONTENT                 equ 206
MULTI_STATUS                    equ 207
ALREADY_REPORTED                equ 208
IM_USED                         equ 226

MULTIPLE_CHOICES                equ 300
MOVED_PERMANENTLY               equ 301
FOUND                           equ 302
SEE_OTHER                       equ 303
NOT_MODIFIED                    equ 304
USE_PROXY                       equ 305
TEMPORARY_REDIRECT              equ 307
PERMANENT_REDIRECT              equ 308

BAD_REQUEST                     equ 400
UNAUTHORIZED                    equ 401
PAYMENT_REQUIRED                equ 402
FORBIDDEN                       equ 403
NOT_FOUND                       equ 404
METHOD_NOT_ALLOWED              equ 405
NOT_ACCEPTABLE                  equ 406
PROXY_AUTHENTICATION_REQUIRED   equ 407
REQUEST_TIMEOUT                 equ 408
CONFLICT                        equ 409
GONE                            equ 410
LENGTH_REQUIRED                 equ 411
PRECONDITION_FAILED             equ 412
PAYLOAD_TOO_LARGE               equ 413
URI_TOO_LONG                    equ 414
UNSUPPORTED_MEDIA_TYPE          equ 415
RANGE_NOT_SATISFIABLE           equ 416
EXPECTATION_FAILED              equ 417
I_AM_A_TEAPOT                   equ 418
MISDIRECTED_REQUEST             equ 421
UNPROCESSABLE_ENTITY            equ 422
LOCKED                          equ 423
FAILED_DEPENDENCY               equ 424
TOO_EARLY                       equ 425
UPGRADE_REQUIRED                equ 426
PRECONDITION_REQUIRED           equ 428
TOO_MANY_REQUESTS               equ 429
REQUEST_HEADER_FIELDS_TOO_LARGE equ 431
UNAVAILABLE_FOR_LEGAL_REASONS   equ 451

INTERNAL_SERVER_ERROR           equ 500
NOT_IMPLEMENTED                 equ 501
BAD_GATEWAY                     equ 502
SERVICE_UNAVAILABLE             equ 503
GATEWAY_TIMEOUT                 equ 504
HTTP_VERSION_NOT_SUPPORTED      equ 505
VARIANT_ALSO_NEGOTIATES         equ 506
INSUFFICIENT_STORAGE            equ 507
LOOP_DETECTED                   equ 508
NOT_EXTENDED                    equ 510
NETWORK_AUTHENTICATION_REQUIRED equ 511

; Status Code Strings
STR_CONTINUE            db "100 Continue", NULL_CHAR
STR_SWITCHING_PROTOCOLS db "101 Switching Protocols", NULL_CHAR
STR_PROCESSING          db "102 Processing", NULL_CHAR
STR_EARLY_HINTS         db "103 Early Hints", NULL_CHAR

STR_OK                              db "200 OK", NULL_CHAR
STR_CREATED                         db "201 Created", NULL_CHAR
STR_ACCEPTED                        db "202 Accepted", NULL_CHAR
STR_NON_AUTHORITATIVE_INFORMATION   db "203 Non-Authoritative Information", NULL_CHAR
STR_NO_CONTENT                      db "204 No Content", NULL_CHAR
STR_RESET_CONTENT                   db "205 Reset Content", NULL_CHAR
STR_PARTIAL_CONTENT                 db "206 Partial Content", NULL_CHAR
STR_MULTI_STATUS                    db "207 Multi-Status", NULL_CHAR
STR_ALREADY_REPORTED                db "208 Already Reported", NULL_CHAR
STR_IM_USED                         db "226 IM Used", NULL_CHAR

STR_MULTIPLE_CHOICES    db "300 Multiple Choices", NULL_CHAR
STR_MOVED_PERMANENTLY   db "301 Moved Permanently", NULL_CHAR
STR_FOUND               db "302 Found", NULL_CHAR
STR_SEE_OTHER           db "303 See Other", NULL_CHAR
STR_NOT_MODIFIED        db "304 Not Modified", NULL_CHAR
STR_USE_PROXY           db "305 Use Proxy", NULL_CHAR
STR_TEMPORARY_REDIRECT  db "307 Temporary Redirect", NULL_CHAR
STR_PERMANENT_REDIRECT  db "308 Permanent Redirect", NULL_CHAR

STR_BAD_REQUEST                     db "400 Bad Request", NULL_CHAR
STR_UNAUTHORIZED                    db "401 Unauthorized", NULL_CHAR
STR_PAYMENT_REQUIRED                db "402 Payment Required", NULL_CHAR
STR_FORBIDDEN                       db "403 Forbidden", NULL_CHAR
STR_NOT_FOUND                       db "404 Not Found", NULL_CHAR
STR_METHOD_NOT_ALLOWED              db "405 Method Not Allowed", NULL_CHAR
STR_NOT_ACCEPTABLE                  db "406 Not Acceptable", NULL_CHAR
STR_PROXY_AUTHENTICATION_REQUIRED   db "407 Proxy Authentication Required", NULL_CHAR
STR_REQUEST_TIMEOUT                 db "408 Request Timeout", NULL_CHAR
STR_CONFLICT                        db "409 Conflict", NULL_CHAR
STR_GONE                            db "410 Gone", NULL_CHAR
STR_LENGTH_REQUIRED                 db "411 Length Required", NULL_CHAR
STR_PRECONDITION_FAILED             db "412 Precondition Failed", NULL_CHAR
STR_PAYLOAD_TOO_LARGE               db "413 Payload Too Large", NULL_CHAR
STR_URI_TOO_LONG                    db "414 URI Too Long", NULL_CHAR
STR_UNSUPPORTED_MEDIA_TYPE          db "415 Unsupported Media Type", NULL_CHAR
STR_RANGE_NOT_SATISFIABLE           db "416 Range Not Satisfiable", NULL_CHAR
STR_EXPECTATION_FAILED              db "417 Expectation Failed", NULL_CHAR
STR_I_AM_A_TEAPOT                   db "418 I'm a teapot", NULL_CHAR
STR_MISDIRECTED_REQUEST             db "421 Misdirected Request", NULL_CHAR
STR_UNPROCESSABLE_ENTITY            db "422 Unprocessable Entity", NULL_CHAR
STR_LOCKED                          db "423 Locked", NULL_CHAR
STR_FAILED_DEPENDENCY               db "424 Failed Dependency", NULL_CHAR
STR_TOO_EARLY                       db "425 Too Early", NULL_CHAR
STR_UPGRADE_REQUIRED                db "426 Upgrade Required", NULL_CHAR
STR_PRECONDITION_REQUIRED           db "428 Precondition Required", NULL_CHAR
STR_TOO_MANY_REQUESTS               db "429 Too Many Requests", NULL_CHAR
STR_REQUEST_HEADER_FIELDS_TOO_LARGE db "431 Request Header Fields Too Large", NULL_CHAR
STR_UNAVAILABLE_FOR_LEGAL_REASONS   db "451 Unavailable For Legal Reasons", NULL_CHAR

STR_INTERNAL_SERVER_ERROR           db "500 Internal Server Error", NULL_CHAR
STR_NOT_IMPLEMENTED                 db "501 Not Implemented", NULL_CHAR
STR_BAD_GATEWAY                     db "502 Bad Gateway", NULL_CHAR
STR_SERVICE_UNAVAILABLE             db "503 Service Unavailable", NULL_CHAR
STR_GATEWAY_TIMEOUT                 db "504 Gateway Timeout", NULL_CHAR
STR_HTTP_VERSION_NOT_SUPPORTED      db "505 HTTP Version Not Supported", NULL_CHAR
STR_VARIANT_ALSO_NEGOTIATES         db "506 Variant Also Negotiates", NULL_CHAR
STR_INSUFFICIENT_STORAGE            db "507 Insufficient Storage", NULL_CHAR
STR_LOOP_DETECTED                   db "508 Loop Detected", NULL_CHAR
STR_NOT_EXTENDED                    db "510 Not Extended", NULL_CHAR
STR_NETWORK_AUTHENTICATION_REQUIRED db "511 Network Authentication Required", NULL_CHAR

section .text
; Returns the string representation of a status code
; @param  rdi: status code (int)
; @return rax: pointer to a string representing the status code
get_status_string:
  ; 1xx informational
  cmp   rdi, CONTINUE
  je    .continue
  cmp   rdi, SWITCHING_PROTOCOLS
  je    .switching_protocols
  cmp   rdi, PROCESSING
  je    .processing
  cmp   rdi, EARLY_HINTS
  je    .early_hints

  ; 2xx success
  cmp   rdi, OK
  je    .ok
  cmp   rdi, CREATED
  je    .created
  cmp   rdi, ACCEPTED
  je    .accepted
  cmp   rdi, NON_AUTHORITATIVE_INFORMATION
  je    .non_authoritative_information
  cmp   rdi, NO_CONTENT
  je    .no_content
  cmp   rdi, RESET_CONTENT
  je    .reset_content
  cmp   rdi, PARTIAL_CONTENT
  je    .partial_content
  cmp   rdi, MULTI_STATUS
  je    .multi_status
  cmp   rdi, ALREADY_REPORTED
  je    .already_reported
  cmp   rdi, IM_USED
  je    .im_used

  ; 3xx redirection
  cmp   rdi, MULTIPLE_CHOICES
  je    .multiple_choices
  cmp   rdi, MOVED_PERMANENTLY
  je    .moved_permanently
  cmp   rdi, FOUND
  je    .found
  cmp   rdi, SEE_OTHER
  je    .see_other
  cmp   rdi, NOT_MODIFIED
  je    .not_modified
  cmp   rdi, USE_PROXY
  je    .use_proxy
  cmp   rdi, TEMPORARY_REDIRECT
  je    .temporary_redirect
  cmp   rdi, PERMANENT_REDIRECT
  je    .permanent_redirect

  ; 4xx client errors
  cmp   rdi, BAD_REQUEST
  je    .bad_request
  cmp   rdi, UNAUTHORIZED
  je    .unauthorized
  cmp   rdi, PAYMENT_REQUIRED
  je    .payment_required
  cmp   rdi, FORBIDDEN
  je    .forbidden
  cmp   rdi, NOT_FOUND
  je    .not_found
  cmp   rdi, METHOD_NOT_ALLOWED
  je    .method_not_allowed
  cmp   rdi, NOT_ACCEPTABLE
  je    .not_acceptable
  cmp   rdi, PROXY_AUTHENTICATION_REQUIRED
  je    .proxy_authentication_required
  cmp   rdi, REQUEST_TIMEOUT
  je    .request_timeout
  cmp   rdi, CONFLICT
  je    .conflict
  cmp   rdi, GONE
  je    .gone
  cmp   rdi, LENGTH_REQUIRED
  je    .length_required
  cmp   rdi, PRECONDITION_FAILED
  je    .precondition_failed
  cmp   rdi, PAYLOAD_TOO_LARGE
  je    .payload_too_large
  cmp   rdi, URI_TOO_LONG
  je    .uri_too_long
  cmp   rdi, UNSUPPORTED_MEDIA_TYPE
  je    .unsupported_media_type
  cmp   rdi, RANGE_NOT_SATISFIABLE
  je    .range_not_satisfiable
  cmp   rdi, EXPECTATION_FAILED
  je    .expectation_failed
  cmp   rdi, I_AM_A_TEAPOT
  je    .i_am_a_teapot
  cmp   rdi, MISDIRECTED_REQUEST
  je    .misdirected_request
  cmp   rdi, UNPROCESSABLE_ENTITY
  je    .unprocessable_entity
  cmp   rdi, LOCKED
  je    .locked
  cmp   rdi, FAILED_DEPENDENCY
  je    .failed_dependency
  cmp   rdi, TOO_EARLY
  je    .too_early
  cmp   rdi, UPGRADE_REQUIRED
  je    .upgrade_required
  cmp   rdi, PRECONDITION_REQUIRED
  je    .precondition_required
  cmp   rdi, TOO_MANY_REQUESTS
  je    .too_many_requests
  cmp   rdi, REQUEST_HEADER_FIELDS_TOO_LARGE
  je    .request_header_fields_too_large
  cmp   rdi, UNAVAILABLE_FOR_LEGAL_REASONS
  je    .unavailable_for_legal_reasons

  ; 5xx server errors
  cmp   rdi, INTERNAL_SERVER_ERROR
  je    .internal_server_error
  cmp   rdi, NOT_IMPLEMENTED
  je    .not_implemented
  cmp   rdi, BAD_GATEWAY
  je    .bad_gateway
  cmp   rdi, SERVICE_UNAVAILABLE
  je    .service_unavailable
  cmp   rdi, GATEWAY_TIMEOUT
  je    .gateway_timeout
  cmp   rdi, HTTP_VERSION_NOT_SUPPORTED
  je    .http_version_not_supported
  cmp   rdi, VARIANT_ALSO_NEGOTIATES
  je    .variant_also_negotiates
  cmp   rdi, INSUFFICIENT_STORAGE
  je    .insufficient_storage
  cmp   rdi, LOOP_DETECTED
  je    .loop_detected
  cmp   rdi, NOT_EXTENDED
  je    .not_extended
  cmp   rdi, NETWORK_AUTHENTICATION_REQUIRED
  je    .network_authentication_required

  ; unknown
  jmp   .error

.continue:
  lea   rax, [REL STR_CONTINUE]
  ret
.switching_protocols:
  lea   rax, [REL STR_SWITCHING_PROTOCOLS]
  ret
.processing:
  lea   rax, [REL STR_PROCESSING]
  ret
.early_hints:
  lea   rax, [REL STR_EARLY_HINTS]
  ret

.ok:
  lea   rax, [REL STR_OK]
  ret
.created:
  lea   rax, [REL STR_CREATED]
  ret
.accepted:
  lea   rax, [REL STR_ACCEPTED]
  ret
.non_authoritative_information:
  lea   rax, [REL STR_NON_AUTHORITATIVE_INFORMATION]
  ret
.no_content:
  lea   rax, [REL STR_NO_CONTENT]
  ret
.reset_content:
  lea   rax, [REL STR_RESET_CONTENT]
  ret
.partial_content:
  lea   rax, [REL STR_PARTIAL_CONTENT]
  ret
.multi_status:
  lea   rax, [REL STR_MULTI_STATUS]
  ret
.already_reported:
  lea   rax, [REL STR_ALREADY_REPORTED]
  ret
.im_used:
  lea   rax, [REL STR_IM_USED]
  ret

.multiple_choices:
  lea   rax, [REL STR_MULTIPLE_CHOICES]
  ret
.moved_permanently:
  lea   rax, [REL STR_MOVED_PERMANENTLY]
  ret
.found:
  lea   rax, [REL STR_FOUND]
  ret
.see_other:
  lea   rax, [REL STR_SEE_OTHER]
  ret
.not_modified:
  lea   rax, [REL STR_NOT_MODIFIED]
  ret
.use_proxy:
  lea   rax, [REL STR_USE_PROXY]
  ret
.temporary_redirect:
  lea   rax, [REL STR_TEMPORARY_REDIRECT]
  ret
.permanent_redirect:
  lea   rax, [REL STR_PERMANENT_REDIRECT]
  ret

.bad_request:
  lea   rax, [REL STR_BAD_REQUEST]
  ret
.unauthorized:
  lea   rax, [REL STR_UNAUTHORIZED]
  ret
.payment_required:
  lea   rax, [REL STR_PAYMENT_REQUIRED]
  ret
.forbidden:
  lea   rax, [REL STR_FORBIDDEN]
  ret
.not_found:
  lea   rax, [REL STR_NOT_FOUND]
  ret
.method_not_allowed:
  lea   rax, [REL STR_METHOD_NOT_ALLOWED]
  ret
.not_acceptable:
  lea   rax, [REL STR_NOT_ACCEPTABLE]
  ret
.proxy_authentication_required:
  lea   rax, [REL STR_PROXY_AUTHENTICATION_REQUIRED]
  ret
.request_timeout:
  lea   rax, [REL STR_REQUEST_TIMEOUT]
  ret
.conflict:
  lea   rax, [REL STR_CONFLICT]
  ret
.gone:
  lea   rax, [REL STR_GONE]
  ret
.length_required:
  lea   rax, [REL STR_LENGTH_REQUIRED]
  ret
.precondition_failed:
  lea   rax, [REL STR_PRECONDITION_FAILED]
  ret
.payload_too_large:
  lea   rax, [REL STR_PAYLOAD_TOO_LARGE]
  ret
.uri_too_long:
  lea   rax, [REL STR_URI_TOO_LONG]
  ret
.unsupported_media_type:
  lea   rax, [REL STR_UNSUPPORTED_MEDIA_TYPE]
  ret
.range_not_satisfiable:
  lea   rax, [REL STR_RANGE_NOT_SATISFIABLE]
  ret
.expectation_failed:
  lea   rax, [REL STR_EXPECTATION_FAILED]
  ret
.i_am_a_teapot:
  lea   rax, [REL STR_I_AM_A_TEAPOT]
  ret
.misdirected_request:
  lea   rax, [REL STR_MISDIRECTED_REQUEST]
  ret
.unprocessable_entity:
  lea   rax, [REL STR_UNPROCESSABLE_ENTITY]
  ret
.locked:
  lea   rax, [REL STR_LOCKED]
  ret
.failed_dependency:
  lea   rax, [REL STR_FAILED_DEPENDENCY]
  ret
.too_early:
  lea   rax, [REL STR_TOO_EARLY]
  ret
.upgrade_required:
  lea   rax, [REL STR_UPGRADE_REQUIRED]
  ret
.precondition_required:
  lea   rax, [REL STR_PRECONDITION_REQUIRED]
  ret
.too_many_requests:
  lea   rax, [REL STR_TOO_MANY_REQUESTS]
  ret
.request_header_fields_too_large:
  lea   rax, [REL STR_REQUEST_HEADER_FIELDS_TOO_LARGE]
  ret
.unavailable_for_legal_reasons:
  lea   rax, [REL STR_UNAVAILABLE_FOR_LEGAL_REASONS]
  ret

.internal_server_error:
  lea   rax, [REL STR_INTERNAL_SERVER_ERROR]
  ret
.not_implemented:
  lea   rax, [REL STR_NOT_IMPLEMENTED]
  ret
.bad_gateway:
  lea   rax, [REL STR_BAD_GATEWAY]
  ret
.service_unavailable:
  lea   rax, [REL STR_SERVICE_UNAVAILABLE]
  ret
.gateway_timeout:
  lea   rax, [REL STR_GATEWAY_TIMEOUT]
  ret
.http_version_not_supported:
  lea   rax, [REL STR_HTTP_VERSION_NOT_SUPPORTED]
  ret
.variant_also_negotiates:
  lea   rax, [REL STR_VARIANT_ALSO_NEGOTIATES]
  ret
.insufficient_storage:
  lea   rax, [REL STR_INSUFFICIENT_STORAGE]
  ret
.loop_detected:
  lea   rax, [REL STR_LOOP_DETECTED]
  ret
.not_extended:
  lea   rax, [REL STR_NOT_EXTENDED]
  ret
.network_authentication_required:
  lea   rax, [REL STR_NETWORK_AUTHENTICATION_REQUIRED]
  ret

.error:
  mov   rax, FAILURE_CODE
  ret

; returns the foreground color of the status code
; @param  rdi: status code (int)
; @param  rax: pointer to the ansi sequence
get_status_code_color:
  cmp   rdi, 100
  jl    .error

  cmp   rdi, 599
  jg    .error

  cmp   rdi, 500
  jge   .red

  cmp   rdi, 400
  jge   .yellow

  cmp   rdi, 300
  jge   .cyan

  cmp   rdi, 200
  jge   .green

  cmp   rdi, 100
  jge   .blue

.red:
  lea   rax, [RED_FG]
  jmp   .return

.yellow:
  lea   rax, [YELLOW_FG]
  jmp   .return

.cyan:
  lea   rax, [CYAN_FG]
  jmp   .return

.green:
  lea   rax, [GREEN_FG]
  jmp   .return

.blue:
  lea   rax, [BLUE_FG]
  jmp   .return

.error:
  lea   rax, [DEFAULT_FG]

.return:
  ret


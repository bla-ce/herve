section .data

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
STR_NO_CONTENT        db "204 No Content", NULL_CHAR
STR_RESET_CONTENT     db "205 Reset Content", NULL_CHAR
STR_PARTIAL_CONTENT   db "206 Partial Content", NULL_CHAR
STR_MULTI_STATUS      db "207 Multi-Status", NULL_CHAR
STR_ALREADY_REPORTED  db "208 Already Reported", NULL_CHAR
STR_IM_USED           db "226 IM Used", NULL_CHAR

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
; @return rax: string status_code
get_status_string:
  cmp   rdi, NO_CONTENT
  je    .no_content

  cmp   rdi, OK
  je    .ok

  cmp   rdi, BAD_REQUEST
  je    .bad_request

  cmp   rdi, NOT_FOUND
  je    .not_found

  cmp   rdi, INTERNAL_SERVER_ERROR
  je    .internal_server_error

  jmp   .error

.no_content:
  lea   rax, [STR_NO_CONTENT]
  ret

.ok:
  lea   rax, [STR_OK]
  ret

.bad_request:
  lea   rax, [STR_BAD_REQUEST]
  ret

.not_found:
  lea   rax, [STR_NOT_FOUND]
  ret

.internal_server_error:
  lea   rax, [STR_INTERNAL_SERVER_ERROR]
  ret

.error:
  mov   rax, FAILURE_CODE
  ret


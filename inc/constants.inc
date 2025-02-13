%ifndef CONSTANTS_INC
%define CONSTANTS_INC

; boolean
TRUE  equ 1
FALSE equ 0

; exit code
SUCCESS_CODE equ 0
FAILURE_CODE equ -1

STDOUT          equ 1
LONG_SIZE       equ 10

; extended ascii
MICRO_FIRST   equ 0xCE
MICRO_SECOND  equ 0xBC

; logs
LOG_PORT_LEN    equ 5 ; port + new line char
LOG_TIME_SIZE   equ 25

; error
ERR_MAX_LEN equ 128

; utils
TO_STRING_MAX equ 7

%endif


%ifndef CONSTANTS_INC
%define CONSTANTS_INC

CLOCK_REALTIME  equ 0
STDOUT          equ 1
LONG_SIZE       equ 10

; socket constants
INADDR_ANY          equ 0
SOCK_STREAM         equ 1
SOL_SOCKET          equ 1
BACKLOG             equ 1
SO_REUSEADDR        equ 2
SO_REUSEPORT        equ 15
AF_INET             equ 2
REQUEST_MAX_LEN     equ 8*1024
MSG_TRUNC           equ 32
METHOD_MAX_LEN      equ 8
ROUTE_MAX_LEN       equ 32
MAX_ROUTES_COUNT    equ 10
STATUS_CODE_LEN     equ 3
RESPONSE_MAX_LEN    equ 32
STRUCT_ROUTE_LEN    equ ROUTE_MAX_LEN + METHOD_MAX_LEN + RESPONSE_MAX_LEN

; syscall values
SYS_READ          equ 0
SYS_WRITE         equ 1
SYS_OPEN          equ 2
SYS_CLOSE         equ 3
SYS_SOCKET        equ 41
SYS_ACCEPT        equ 43
SYS_SENDTO        equ 44
SYS_RECVFROM      equ 45
SYS_BIND          equ 49
SYS_LISTEN        equ 50
SYS_SETSOCKOPT    equ 54
SYS_EXIT          equ 60
SYS_CLOCK_GETTIME equ 228

; ascii
NULL_CHAR       equ 0
END_OF_TEXT     equ 3
LINE_FEED       equ 10
CARRIAGE_RETURN equ 13
SPACE           equ 32
NUMBER_SIGN     equ 35
HYPHEN          equ 45
DOT             equ 46
SLASH           equ 47
ZERO_CHAR       equ 48
QUESTION_MARK   equ 63
LOW_N_CHAR      equ 110
LOW_S_CHAR      equ 115

; file constants
O_RDONLY      equ 0
FILE_MAX_LEN  equ 8*1024

; exit code
SUCCESS_CODE equ 0
FAILURE_CODE equ -1

%endif


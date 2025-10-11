%include "boeuf.inc"
%include "config.inc"
%include "logan.inc"
%include "malloc.inc"
%include "utils.inc"

%include "hv_auth.inc"
%include "hv_encoding.inc"
%include "hv_http.inc"
%include "hv_middlewares.inc"
%include "hv_server.inc"
%include "hv_socket.inc"

section .bss
section .data

HERVE_LOGO db  " _____________________________________________ ",LINE_FEED
           db  "|    _____   _____   _____   _____   _____    |",LINE_FEED
           db  "|   |  |  | |   __| | __  | |  |  | |   __|   |",LINE_FEED
           db  "|   |     | |   __| |    -| |  |  | |   __|   |",LINE_FEED
           db  "|   |__|__| |_____| |__|__|  \___/  |_____|   |",LINE_FEED
           db  "|_____________________________________________|",LINE_FEED, NULL_CHAR

section .data
  
  client_struct:
    client_fd       dq 0  
    client_sockaddr dq 0
  client_struct_end:

  CLIENT_STRUCT_LEN equ client_struct_end - client_struct

  CLIENT_OFF_CLIENT_FD equ 0x0
  CLIENT_OFF_SOCKADDR  equ CLIENT_OFF_CLIENT_FD + 0x8


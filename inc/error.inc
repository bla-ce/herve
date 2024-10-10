%ifndef ERROR_INC
%define ERROR_INC

perror:
  ; rdi -> error message 
  mov   rsi, 0 ; we don't know the length
  call  print

  ret

%endif


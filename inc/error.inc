%ifndef ERROR_INC
%define ERROR_INC

print_error:
  ; rdi -> error message 
  mov   rsi, 0 ; we don't know the length
  call  debug
  ret

%endif


%ifndef ERROR_INC
%define ERROR_INC

print_error:
  ; rdi -> error message 
  call  debug
  ret

%endif


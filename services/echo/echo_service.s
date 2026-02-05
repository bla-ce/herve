section .data

echo_service_t:
  .id         db 0  ; set by the central server
  .name       dq 0  ; set by the caller
  .status     dq 0  ; set by the central server
  .port       dq 0  ; set by the central server
  .type       dq service_types.echo
  .register   dq echo_service_register
  .unregister dq echo_service_unregister
  .start      dq echo_service_start
  .stop       dq echo_service_stop
  .next       dq 0  ; set by the central server
echo_service_t_end:

section .text

echo_service_register:
  ret

echo_service_unregister:
  ret

echo_service_start:
  ret

echo_service_stop:
  ret

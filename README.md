# Implementation of a HTTP Server Library in Netwide Assembly

> [!WARNING]
> This library is for educational or experimental purposes only.
> It is NOT suitable yet for production use

## Documentation (Work In Progress)

https://herve-asm.gitbook.io/herve-docs

## Overview

Herve is a lightweight, high-performance HTTP server library written entirely in Netwide Assembly (NASM) for x86 systems. It provides a simple API for handling HTTP requests and responses, making it easier for developers to build web applications in pure Assembly without worrying about low-level network operations.

## Features

- **Linux x86 Only**: Designed exclusively for Linux on x86 architecture.
- **Built-in Utility Functions**: Provides low-level functions allowing users to focus on their application logic and speed up Assembly development.
- **Tiny executable**: ~25KB (stripped)
- **Zero external dependencies**: No libc required
- **High Performance**: Optimized for speed with custom memory management.
- **Custom Malloc Implementation**: Efficient memory allocation using custom malloc.
- **HTTP/1.1 Support**: Support for HTTP/1.1 requests and responses.
- **Request Parsing**: Built-in HTTP request parser.
- **Configurable Port Number**: Users can specify the listening port.
- **Custom Request Handlers**: Users can define custom handlers for different request routes.
- **Middleware Support**: Users can add middlewares to extend request processing.
- **Static File Serving**: Supports serving static files and entire directories.
- **Memory Management**: Built-in memory management with automatic cleanup.
- **Hash Table Implementation**: Users can use hash tables
- **Basic Template Engine**: Built-in basic template engine
- **Support Query Parameters**: Is able to read and parse query parameters
- **Basic Authorization**: Supports Basic Auth

## Performance

Test Configuration:
- **Test Duration:** 30 seconds
- **Test URL:** /index
- **Benchmark Tool:** wrk
- **Configuration:** 1 thread, 1 connection

Result:
- **Latency:** 18.22us

Latency Distribution:
- **50%:** 13.00us
- **75%:** 17.00us
- **90%:** 22.00us
- **99%:** 93.00us

## Example

```asm
global _start

%include "herve.inc"

section .text

echo:
  sub   rsp, 0x8

  mov   [rsp], rdi

  call  get_ctx_request
  cmp   rax, 0
  jl    .error

  mov   rdi, rax
  call  get_request_body
  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, OK
  mov   rdx, rax
  call  send_string

  jmp   .return

.error:
  mov   rdi, [rsp] 
  mov   rsi, INTERNAL_SERVER_ERROR
  call  send_no_content

  mov   rax, FAILURE_CODE

.return:
  add   rsp, 0x8
  ret

_start:
  sub   rsp, 0x8

  mov   rdi, 1337
  call  server_init
  cmp   rax, 0
  jl    error

  mov   [rsp], rax 

  mov   rdi, rax
  lea   rsi, [POST]
  lea   rdx, [wildcard_url]
  mov   rcx, echo
  call  add_route
  cmp   rax, 0
  jl    error

  mov   rdi, [rsp]
  call  run_server
  cmp   rax, 0
  jl    error

  mov   rax, SYS_EXIT
  mov   rdi, SUCCESS_CODE
  syscall

error:
  mov   rax, SYS_EXIT
  mov   rdi, FAILURE_CODE
  syscall

section .data
  wildcard_url  db "*", NULL_CHAR
```

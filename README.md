# Implementation of a HTTP Server Library in Netwide Assembly

> [!WARNING]
> This library is for educational or experimental purposes only.
> It is NOT suitable yet for production use

## Official Documentation will be available soon

## Overview

This project is a lightweight, high-performance HTTP server library written entirely in Netwide Assembly (NASM) for x86 systems. It provides a simple API for handling HTTP requests and responses, making it easier for developers to build web applications in pure Assembly without worrying about low-level network operations.

## Features

- **Linux x86 Only**: Designed exclusively for Linux on x86 architecture.
- **Lightweight**: Minimal memory footprint and no external dependencies.
- **High Performance**: Optimized for speed with custom memory management.
- **Custom Malloc Implementation**: Efficient memory allocation using custom malloc.
- **HTTP/1.1 Support**: Support for HTTP/1.1 requests and responses.
- **Request Parsing**: Built-in HTTP request parser.
- **Configurable Port Number**: Users can specify the listening port.
- **Custom Request Handlers**: Users can define custom handlers for different request routes.
- **Middleware Support**: Users can add middleware to extend request processing.
- **Static File Serving**: Supports serving static files and entire directories.
- **Memory Management**: Built-in memory management with automatic cleanup.
- **Hash Table Implementation**: Users can use hash tables
- **Support Query Parameters**: Is able to read and parse query parameters

## Example

```asm
global  _start

%include "bytasm.inc"

section .text
middleware:
  lea   rdi, [ok_msg]  
  mov   rsi, 0
  call  println

  ret

test_no_content:
  call  send_no_content
  ret

test_string:
  mov   rsi, OK  
  lea   rdx, [ok_msg]
  call  send_string
  ret

test_static:
  lea   rsi, [index_path]
  call  send_static_file
  ret

_start:
  sub   rsp, 0x8

  mov   rdi, 1337
  call  server_init
  cmp   rax, 0
  jl    .error

  mov   [rsp], rax

  mov   rdi, rax
  call  get_server_sockfd

  cmp   rax, 0
  jl    .error

  mov   qword [sockfd], rax

  ; add no content route
  mov   rdi, [rsp]
  lea   rsi, [GET]
  lea   rdx, [root_url]
  mov   rcx, test_no_content
  call  add_route

  cmp   rax, 0
  jl    .error

  ; add health route
  mov   rdi, [rsp]
  lea   rsi, [GET]
  lea   rdx, [health_url]
  mov   rcx, test_string
  call  add_route

  ; add health route
  mov   rdi, [rsp]
  lea   rsi, [GET]
  lea   rdx, [index_url]
  mov   rcx, test_static
  call  add_route

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, middleware
  call  add_middleware

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  mov   rsi, middleware
  call  add_middleware

  cmp   rax, 0
  jl    .error

  mov   rdi, [rsp]
  call  run_server

  add   rsp, 0x8

  mov   rax, SYS_EXIT
  mov   rdi, SUCCESS_CODE
  syscall

.error:
  add   rsp, 0x8

  mov   rax, SYS_EXIT
  mov   rdi, FAILURE_CODE
  syscall

section .bss

section .data
  sockfd  dq 0

  root_url    db "/", NULL_CHAR
  health_url  db "/health", NULL_CHAR
  index_url  db "/index", NULL_CHAR

  index_path db "examples/views/index.html", NULL_CHAR

  ok_msg db "middlewares ok", NULL_CHAR
```

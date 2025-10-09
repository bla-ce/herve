# Herve

## What is Herve?

Herve is a high-performance HTTP Server Library written entirely in Assembly for x86 systems.

## Why Herve has been built and why would you use it?

To gain a deeper knowledge of computer architecture and how the CPU works at the instruction level, I wanted to build projects in Assembly.

But let's be real, spending hours to just write basic functions like `to_string` or `strlen` is not the most exciting or rewarding thing.

The goal of this library is to provide a thorough yet simple API to allow developers to write exciting Assembly projects with a webserver without spending months building the underlying webserver and network operations.

No, I want developers to be able to spin up a webserver with 70 lines of Assembly and focus on the rest of the project.

After one year and ~17,000 lines of Assembly code, we are still far away from a production ready state, but we'll get there.

(of course, this library is also intended to show off).

## Install

Copy the `include/` and `lib/` directories into your project tree and use the `Makefile` template in `examples/` to build your project with Herve.

```bash
git clone https://github.com/bla-ce/herve
cd herve
cp -r include <project-path>
cp -r lib <project-path>
cd <project-path>
make
```

**Incoming:**
Use `herve` as a static or shared library.

## Features

### Performance

Because of its implementation in Assembly, `Herve` is incredibly fast but it is not yet written or optimized for performance.

`Herve` is currently written to be as readable and understainable  as possible compromising performance.

### Custom Request Handler

You can write custom request handlers for each route you define.

Each handler will receive the context structure as an argument so that you have all the necessary information about the request.

### Middlewares

When writing a webserver, you might want to add custom middlewares for specific groups of routes like authentication, reverse-proxy, request logs etc...

To avoid writing these functions for every route, you can add middlewares that can be executed before or after the request handler. These middlewares are stored as a linked list.

`Herve` provides some middlewares, including a custom request logger and a proxy middleware.

### Custom malloc

To make my life easier and reduce variables scope, I have implemented my own malloc and free functions. More info can be found [here](https://github.com/bla-ce/unstack).

You can access the globals `mallocd`, `freed` or `mmapd` to make sure you don't have memory leaks.

### Send static files

`Herve` supports various `send_` functions. Devs can send responses without content, with a string or send static files. The `add_dir_route` function allows devs to define a route for each file in the directory as long as the format is supported.

### Template Engine

A basic template engine has been implemented so that devs can return dynamic content, passing a hash table as an argument. The library also contains a custom hash table implementation to make things easier. 

At the moment, only string keys and values are supported which should be enough for most use cases.

### Basic Auth

Basic auth can be used with `Herve`, this is the simplest and of course one of the least robust way to provide authentication but I am hoping to implement other authentication methods in the future as middlewares.

### Grouping

Routes with a common prefix can be grouped inside a `group` structure with specific middlewares.

### Dynamic Route Parameters

A route can be defined with dynamic paramater such as `/api/v1/events/:id`. This feature allows for flexibility in supporting varying parameters within the defined route.

## Quick Start

Here is an example of an echo server built with `Herve`:

```assembly
global _start

%include "herve.inc"
%include "os.inc"

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

  ; initialise server with port 1337
  mov   rdi, 1337
  call  server_init
  cmp   rax, 0
  jl    error

  mov   [rsp], rax 

  ; add a new POST route
  mov   rdi, [rsp]
  lea   rsi, [POST]
  lea   rdx, [wildcard_url]
  mov   rcx, echo
  xor   r8, r8
  call  add_route
  cmp   rax, 0
  jl    error

  ; run the server
  mov   rdi, [rsp]
  call  run_server
  cmp   rax, 0
  jl    error

  mov   rdi, SUCCESS_CODE
  call  exit

error:
  mov   rdi, FAILURE_CODE
  call  exit
  
section .data
  wildcard_url  db "*", NULL_CHAR
```

## Benchmark

TODO

## Documentation

TODO

## More to come



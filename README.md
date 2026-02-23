# Herve

Herve is a service orchestration platform written in x86-64 Assembly. It allows you to spin up, manage, and proxy services through a unified interface.

> [!WARNING]
> This project is intended for educational purposes only and is not yet ready for production use. It's Assembly, not sure what you were expecting to be honest.

## Overview

Think of Herve as a minimalist alternative to cloud service platforms. It acts as the central hub that:

- **Registers and manages services** - Spin up built-in or custom services
- **Proxies requests** - Routes traffic to the appropriate service
- **Enforces contracts** - Ensures all services comply with a defined interface

You can use the built-in services or create your own. As long as your service implements the Herve contract, it can be registered and managed like any other.

## Background

To gain a deeper knowledge of computer architecture, networking and how the CPU works at the instruction level, I wanted to build projects in Assembly, so I decided to write a HTTP server library.

After one year and ~28,500 lines of Assembly code, I decided to move away from a library to an actual project. The goal: a cloud platform in x86 to compete (lol) against AWS, GCP, Azure and so on.

## Building

Requires NASM and GNU LD.

```bash
make
```

The binary will be available at `bin/herve`.

## Running

```bash
./bin/herve
```

By default, Herve listens on port 5000, but you can update the listening port with the `PORT` environment variable.

## Service Management API

### Register a service

```bash
curl -X POST http://localhost:5000/services/register \
  -d "name=my-service&type=echo"
```

### List services

```bash
curl http://localhost:5000/services
```

Returns all registered services with their id, name, type, and status.

### Unregister a service

```bash
curl -X POST http://localhost:5000/services/:id/unregister
```

### Start a service

```bash
curl -X POST http://localhost:5000/services/:id/start
```

### Stop a service

```bash
curl -X POST http://localhost:5000/services/:id/stop
```

## Creating Custom Services

Custom services must implement the Herve service contract. A service is defined by:

| Field      | Description                                      | Need to be defined        |
|------------|--------------------------------------------------|---------------------------|
| id         | Auto-generated service identifier                | no                        |
| name       | Name of the service                              | no                        |
| status     | Current status of the service                    | no                        |
| type       | Type of the service                              | yes                       |
| register   | Function pointer to register the service         | yes                       |
| unregister | Function pointer to unregister the service       | yes                       |
| start      | Function pointer to start the service            | yes                       |
| stop       | Function pointer to stop the service             | yes                       |
| group      | Pointer to the server group with all the routes  | no                        |
| next       | Pointer to the next service (linked list)        | no                        |

## Project Structure

```
herve/
├── src/           # Main application source
│   ├── herve.s    # Entry point and service manager
│   └── services/  # Service registration logic
├── svc_impl/      # Built-in service implementations
├── include/       # Public API headers
│   ├── server/    # Socket, routing, context
│   ├── http/      # Request/response handling
│   ├── http_models/   # CRUD endpoint generation
│   ├── auth/      # Authentication
│   └── middlewares/   # Logger, proxy, CSRF
├── lib/           # Core libraries
│   ├── malloc/    # Memory allocator
│   ├── net/       # Sockets, epoll, select
│   ├── encoding/  # JSON, Base64
│   ├── hash_table/    # Key-value storage
│   ├── model/     # Data model system
│   ├── utils/     # Strings, linked lists, arrays
│   ├── logan/     # Logging
│   └── boeuf/     # Dynamic buffers
├── examples/      # Working examples
└── tests/         # Unit tests
```

## Examples

The `examples/` directory contains demonstrations from when Herve was a library. They showcase the underlying HTTP server capabilities:

- **hello-world** - Minimal server setup
- **models** - Data model CRUD operations
- **groups** - Route grouping and prefixes
- **echo** - Echo server
- **proxy** - Reverse proxy configuration
- **static-content** - Static file serving

---

And because apparently, now, it has to be mentioned, absolutely no AI was used, but this project probably served to train AI :)

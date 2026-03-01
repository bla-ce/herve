# Herve

Herve is a service orchestration platform written in x86-64 Assembly. It allows you to spin up, manage, and proxy services through a unified interface.

> [!WARNING]
> This project is intended for educational purposes only and is not yet ready for production use. It's Assembly, not sure what you were expecting to be honest.

## Overview

Think of Herve as a minimalist alternative to cloud service platforms. It acts as the central hub that:

- **Registers and manages services** - Spin up built-in or custom services
- **Proxies requests** - Routes traffic to the appropriate service
- **Enforces contracts** - Ensures all services comply with a defined interface

You can use the built-in services or create your own. As long as your service implements the Herve contract, it can be registered and managed like any other. Herve provides a comprehensive public API to help you develop your own services.

## Background

To gain a deeper knowledge of computer architecture, networking and how the CPU works at the instruction level, I wanted to build projects in Assembly, so I decided to write a HTTP server library.

After one year and ~30,000 lines of Assembly code, I decided to move away from a library to an actual project. The goal: a cloud platform in x86 to compete (lol) against AWS, GCP, Azure and so on.

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

### Authentication

Herve requires basic authentication for the Service Management API. Set the following environment variables:

```bash
AUTH_ADMIN_ID=admin
AUTH_ADMIN_SECRET=password
```

All requests to the API must include the `Authorization` header with valid Basic Auth credentials.

> [!NOTE]
> Yes, I know this implementation is not secure (yet). But hey, I'm implementing authentication in raw Assembly, not centering a div in React. We'll get to the fancy stuff eventually (SPOIL: I am implementing bcrypt in x86).

## Service Management API

### Register a service

```bash
curl -X POST http://localhost:5000/services/register \
  -u admin:password \
  -d "name=my-service&type=echo"
```

### List services

```bash
curl -u admin:password http://localhost:5000/services
```

Returns all registered services with their uuid, name, type, and status.

### Unregister a service

```bash
curl -X POST -u admin:password http://localhost:5000/services/:uuid/unregister
```

### Start a service

```bash
curl -X POST -u admin:password http://localhost:5000/services/:uuid/start
```

### Stop a service

```bash
curl -X POST -u admin:password http://localhost:5000/services/:uuid/stop
```

## Creating Custom Services

Custom services must implement the Herve service contract. A service is defined by:

| Field      | Description                                      | Need to be defined        |
|------------|--------------------------------------------------|---------------------------|
| uuid       | Auto-generated service identifier (uuid v4)      | no                        |
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
├── src/            # Main application source
│   ├── herve.s     # Entry point and service manager
│   └── services/   # Service registration logic
├── svc_impl/   # Built-in service implementations
├── include/
│   ├── server/         # Socket, routing, context
│   ├── http/           # Request/response handling
│   ├── http_models/    # CRUD endpoint generation
│   ├── auth/           # Authentication
│   └── middlewares/    # Logger, proxy, CSRF
├── lib/            # Core libraries
│   ├── malloc/     # Memory allocator
│   ├── net/        # Sockets, epoll, select
│   ├── encoding/   # JSON, Base64
│   ├── hash_table/ # Key-value storage
│   ├── model/      # Data model system
│   ├── utils/      # Strings, linked lists, arrays
│   ├── logan/      # Logging
│   ├── uuid/       # UUID generation
│   └── boeuf/      # Dynamic buffers
├── examples/   # Working examples
└── tests/  # Unit tests
```

## Examples

The `examples/` directory contains demonstrations from when Herve was a library. They showcase the underlying HTTP server capabilities and can be used as reference to build your own service:

- **hello-world** - Minimal server setup
- **models** - Data model CRUD operations
- **groups** - Route grouping and prefixes
- **echo** - Echo server
- **proxy** - Reverse proxy configuration
- **static-content** - Static file serving

---

And because apparently, now, it has to be mentioned, absolutely no AI was used, but this project probably served to train AI :)

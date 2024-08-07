# Implementation of a HTTP Server Library in Netwide Assembly

## Quickstart
To use this library, follow the steps below:

### Step 1: Create a Server File
Create a new assembly file named server.s:

``` bash
touch server.s
```

### Step 2: Import the Library
Include the server library in your code by adding the following line:

``` assembly
%include "server.inc"
```

### Step 3: Define Routes
Define your routes inside the .data section:

``` assembly
api_v1_route      db "/api/v1", NULL_CHAR
api_v1_route_len  equ $ - api_v1_route

index_route       db "/index", NULL_CHAR
index_route_len   equ $ - index_route
```

### Step 4: Initialize the Server
Initialize the server by specifying the port number:

``` assembly
server_init 1337
mov   qword [sockfd], rax
```

### Step 5: Add Routes
Add routes to the server:

``` assembly
add_route http_get, http_get_len, api_v1_route, api_v1_route_len
add_route http_get, http_get_len, index_route, index_route_len
```

### Step 6: Run the Server
Run the server:

``` assembly
run_server qword [sockfd]
```

### Step 7: Stop the Server
To stop the server, use:

``` assembly
shutdown
```

Full Example Code

Here is the complete code for your server.s file:

``` assembly
global  _start

%include "server.inc"

section .text
_start:
    server_init 1337
    mov   qword [sockfd], rax

    add_route http_get, http_get_len, api_v1_route, api_v1_route_len
    add_route http_get, http_get_len, index_route, index_route_len

    run_server qword [sockfd]

    shutdown

section .data
sockfd            dq 0

api_v1_route      db "/api/v1", NULL_CHAR
api_v1_route_len  equ $ - api_v1_route

index_route       db "/index", NULL_CHAR
index_route_len   equ $ - index_route
``` 

### Step 8: Assemble and Link the Code
To assemble and link the code, run:

``` bash
make
```

### Step 9: Run the Server
To run the server, use:

``` bash
./server
```

### Step 10: Test the Routes
You can test the routes using the test.sh script. 

Make sure to configure it by adding the routes and expected HTTP status codes. Then, make the script executable and run it:

``` bash
chmod +x test.sh
./test.sh
```

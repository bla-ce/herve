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
%include "bytasm.inc"
```

### Step 3: Define Routes
Define your routes inside the .data section:

``` assembly
index_route   db "/index", NULL_CHAR
health_route  db "/health", NULL_CHAR
```

### Step 4: Initialize the Server
Initialize the server by specifying the port number:

``` assembly
server_init 1337
mov   qword [sockfd], rax
```

### Step 5: Add Routes
Add routes to the server, associating a method:
add_route function supports zero arguments callback function
callback function has the request object inside rsi

``` assembly
add_route GET, index_route, print_hello 
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

%include "bytasm.inc"

section .text
print_hello:
    mov   rax, 1
    mo    rdi, 1
    lea   rsi, [hello]
    mov   rdx, len
    syscall

_start:
    server_init 1337
    mov   qword [sockfd], rax

    disallow_method CONNECT

    add_route GET, index_route, print_hello

    run_server qword [sockfd]

    shutdown

section .data
    sockfd  dq 0
   
    index_route db "/index", NULL_CHAR

    hello db "Hello, World", LINE_FEED
    len   equ $ - hello
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

# boeuf - Dynamic Buffer in Assembly

**boeuf** is a library used to create and manage dynamic buffers in Netwide Assembly. Dynamic buffers are referred as `boeuf` in this library.

## Usage

**Create a boeuf buffer**

```assembly
mov     rdi, str        ; pointer to the string
call    boeuf_create
cmp     rax, 0
jl      .error
```

**Append string to a boeuf buffer**

```assembly
mov     rdi, [boeuf]    ; pointer to the boeuf
mov     rsi, str
call    boeuf_append
cmp     rax, 0
jl      .error

mov     [boeuf], rax    ; don't forget to save the new pointer of the boeuf
```

```assembly
mov     rdi, [boeuf]    ; pointer to the boeuf
mov     rsi, str
mov     rdx, 4          ; the length should be lower than the length of the string
call    boeuf_nappend
cmp     rax, 0
jl      .error

mov     [boeuf], rax    ; don't forget to save the new pointer of the boeuf
```

**Reset boeuf buffer**

```assembly
mov     rdi, [boeuf]    ; pointer to the boeuf
call    boeuf_reset
cmp     rax, 0
jl      .error
```

**Get length of boeuf and set a new (smaller one)**

```assembly
mov     rdi, [boeuf]    ; pointer to the boeuf
call    boeuf_len
cmp     rax, 0
jl      .error

mov     rdi, [boeuf]
mov     rsi, 5          ; assume that the length is smaller
call    boeuf_set_len
cmp     rax, 0
jl      .error
```

**Free the boeuf**

```assembly
mov     rdi, [boeuf]      ; pointer to the boeuf
call    boeuf_free
cmp     rax, 0
jl      .error
```

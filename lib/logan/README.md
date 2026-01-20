# Logan

Logan is a logger for Assembly applications.

It supports the following log levels:

* TRACE
* DEBUG
* LOG
* INFO
* WARN
* ERROR
* FATAL

*If you wish, you can change the logging levels by modifying the constants and prefixes in the file. Please ensure that you also update the functions linked to these levels*.

## Requirements

To use Logan logger, you will need to import `/lib` in your projects [/lib](https://github.com/bla-ce/herve/lib)

## Usage

**Initialise Logan**

To initialise a new Logan logger, include the file and run the `logan_init` function.

It returns a pointer to the Logan struct on success or -1 on failure. Make sure you check the return value of the function.

```assembly
%include "logan.inc"

call    logan_init
cmp     rax, 0
jl      .error
```

This will initialise the Logan logger with colors and timestamp enabled and padding disabled. The default output will be `STDOUT_FILENO` and all levels will be printed by default.

**Customise Logan**

In order to customise Logan's behavior, you can use the getter and setter functions to enable or disable colors, timestamp, set the minimum log level to be printed or the output file descriptor.

```assembly
mov     rdi, logan_struct
call    logan_enable_colors
cmp     rax, 0
jl      .error

mov     rdi, logan_struct
call    logan_disable_prefixes
cmp     rax, 0
jl      .error

mov     rdi, logan_struct
call    logan_get_output
cmp     rax, 0
jl      .error

; rax contains the output file descriptor
```

**Free Logan**

To free a Logan logger, simply run the `logan_free` function.
It returns 0 on success and -1 on failure.

```assembly
mov     rdi, logan_struct   ; pointer to the logan struct
call    logan_free
cmp     rax, 0
jl      .error
```

**Logs**

You can log messages by passing a pointer to the Logan struct and a pointer to the log message. **Note that the string MUST be null terminated**.

If you wish to log a string with a new line, you can use the same functions with the `ln` suffix.

Fatal logs will close the program with -1.

```assembly
mov     rdi, logan_struct   ; pointer to logan struct
mov     rsi, msg            ; pointer to the message
call    log_error           ; [ERROR] msg
cmp     rax, 0
jl      .error
```

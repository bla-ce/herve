# Logan

Logan is a logger for Assembly applications.

It can be used outside this library by copying and pasting the `logan.inc` file.

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

To use Logan logger, you will need [malloc](https://github.com/bla-ce/unstack).

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
Currently, log functions are not linked to the Logan structure, but this will change soon.

For now, you can simply call the appropriate function (i.e. `log_info`) and passing a pointer to the string to be printed. **Note that the string MUST be null terminated**.

If you wish to log a string with a new line, you can use the same functions with the `ln` suffix.

Fatal logs will close the program with -1.

```assembly
mov     rdi, msg    ; pointer to the message
call    log_error   ; [ERROR] msg
cmp     rax, 0
jl      .error
```

## More to come...

- [ ] Update logs format and style
- [ ] Link log functions to the Logan struct
- [ ] Pass a hash table to log functions to add more information
...


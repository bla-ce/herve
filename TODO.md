- [x] send string should duplicate the string before storing response body
- [x] curl -X HEHEHEHEHEHE http://localhost:1337/ seg fault after 3 req
- [x] Template might have a bug, not null terminated?
- [x] all blocks are free
- [x] Template segfault somewhere

- [ ] use getters and setters function with offsets

- [ ] variables should not be mallocd inside functions, user is responsible of memory allocation
- [ ] if invalid argument and error should free, unexpected behavior

- [ ] Routing with Parameters
    - [ ] Would it require new algorithm for routing?

- [ ] Reduce dynamic allocation in favor of stack allocation


- [ ] Alloc one large ctx for each request? 
- [ ] create error codes
- [ ] when freeing a chunk and an error happens later, make sure its not freed twice
- [ ] Make files independent
- [ ] HUUUUUUGE EPIIIIIC CHANGE: not a library anymore?

- [ ] Critical errors should print something
- [ ] Error management
- [ ] Testing and CI pipelines
- [ ] Add rest of the attributes for cookie struct
- [ ] Implement other load balancing algorithm
- [ ] File optimisation
- [ ] Config files
- [ ] Perf optimisation
- [ ] JSON/XML Binding and Validation
- [ ] Database Integration Middleware
- [ ] Compression Middleware
- [ ] Rate Limiting
- [ ] CORS Middleware
- [ ] Request Timeout and Graceful Shutdown
- [ ] Session Management
- [ ] Metrics and Health Checks

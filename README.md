# SteEZ (Steven-NA EZ library)

## Todo
- [x] Add strng_view
    - [ ] Write all the procs I want
- [ ] Test coverage
    - [ ] Deque
        - [ ] Create
        - [ ] enqueue
        - [ ] pop
        - [ ] ts create
        - [ ] ts enqueue
        - [ ] ts pop
    - [ ] logging
    - [ ] slidingwindow
        - [ ] create
        - [ ] insert
    - [ ] string
        - [ ] strng_t
        - [ ] strng_view_t
    - [ ] thread_pool
    - [ ] vec2sw
    - [ ] hashmap
- [x] Stupid bug
    - In smart arena, when I create a scratch arena and allocate 10 bytes for a char pointer and change them all to non '\0' then end the arena, then create another scratch arena on the same global scratch, allocate 8 bytes and change them all to non '\0', this can not be used as a null terminated string because the byte after 8 (9 in the original buffer) is not a '\0'.
    - Verdict: Leave it up to the user to allocate strlen+1 bytes.

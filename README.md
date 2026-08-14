# SteEZ (Steven-NA EZ library)

## Todo
- [x] Add allocation checks and trace log calls (disabled with NLOG_TRACE)
    - [x] deque
    - [x] ez_arena
    - [x] smrt_arena
    - [x] slidingwindow
    - [x] string
    - [x] threadpool
    - [x] vec2sw
    - [x] wav
- [x] Add strng_view
    - want start, end, max_len, char*
- [x] Make status return values consistent. i64, 0 success, -1 fail, >0 info if necessary
    - [x] Deque enqueue (both)
    - [x] smrt_arena_pop_to_mark
    - [x] strng_set
    - [x] write_wav_file
- [ ] Finish thread_pool
    - [ ] make some tests for it
    - [x] base implementation done
        - [x] Note: there is a crash that occurs regularly, but not when running in debug, (a junk data pointer being passed arond that is zeroed out in debug?)
            - Race condition fixed in commit d3e8a23

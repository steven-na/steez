# SteEZ (Steven-NA EZ library)

## Todo
- [ ] Add trace log calls, disabled with NLOG_TRACE
- [ ] Add strng_view
    - want start, end, max_len, char*
- [ ] Make status return values consistent. i64, 0 success, -1 fail, >0 info if necessary
    - [ ] Deque enqueue (both)
    - [ ] smrt_arena_pop_to_mark
    - [ ] strng_set
    - [ ] write_wav_file
- [ ] Finish thread_pool
    - [ ] make some tests for it
    - [x] base implementation done
        - Note: there is a crash that occurs regularly, but not when running in debug, (a junk data pointer being passed arond that is zeroed out in debug?)

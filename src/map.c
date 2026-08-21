#include "map.h"
#include "common.h"
#include "smrt_arena.h"

#include "../include/a5hash.h"

#include <immintrin.h>
#include <stdint.h>
#include <string.h>

#define ENTRY_TOMBSTONE 0b00000001
#define ENTRY_EMPTY     0b00000000
#define ENTRY_MASK      0b11111110

typedef struct {
    void const *key;
    u64 keylen;
} map_key_entry_t;

typedef struct _map_t {
    u64  table_size;
    u8 *hashlookup;
    u64 *hashes;
    map_key_entry_t *keys;
    void **values;
} _map_t;

map_t map_create(smrt_arena_t *arena, u64 capacity) {
    map_t map = smrt_arena_push(arena, sizeof(_map_t), true);

    map->hashes     = SMRTA_ALLOC_ARRAY(arena, u64, capacity);
    map->hashlookup = SMRTA_ALLOC_ARRAY(arena, u8, capacity);
    map->keys       = SMRTA_ALLOC_ARRAY(arena, map_key_entry_t, capacity);
    map->values     = SMRTA_ALLOC_ARRAY(arena, void*, capacity);
    map->table_size = capacity;

    return map;
}

i32 map_insert(map_t const map, u8 const *key, u64 keylen, void *value) {
    u64 h = a5hash128(key, keylen, 0, NULL);
    u8 h_lookup = h >> (64-8) & ENTRY_MASK;

    u64 start = h % map->table_size;
    i64 first_tombstone = -1;
    u64 tsize = map->table_size;
    for (u64 i = start; i != start - 1; i=(i+1)%tsize) {
        u8 h_lookupcmp = map->hashlookup[i];
        if (h_lookupcmp == ENTRY_EMPTY) {
            if (first_tombstone != -1) i = first_tombstone;

            map->hashes[i] = h;
            map->hashlookup[i] = h_lookup;
            map->keys[i] = (map_key_entry_t){ .key=key, .keylen=keylen };
            map->values[i] = value;
            return 0;
        }

        if (first_tombstone == -1 &&
            h_lookupcmp == ENTRY_TOMBSTONE) {
            first_tombstone = i;
            continue;
        }

        if (
            h_lookupcmp == h_lookup       &&
            keylen == map->keys[i].keylen &&
            h == map->hashes[i]           &&
           (memcmp(key, map->keys[i].key, keylen) == 0)
        ) {
            map->keys[i].key=key;
            map->values[i]=value;
        }

    }
    return -1;
}

i32 map_delete(map_t const map, u8 const *key, u64 keylen) {
    u64 h = a5hash128(key, keylen, 0, NULL);
    u8 h_lookup = h >> (64-8) & ENTRY_MASK;

    u64 start = h % map->table_size;
    u64 tsize = map->table_size;
    for (u64 i = start; i != start - 1; i=(i+1)%tsize) {
        u8 h_lookupcmp = map->hashlookup[i];
        if (h_lookupcmp == ENTRY_EMPTY) return -1;
        if (h_lookupcmp == ENTRY_TOMBSTONE) continue;

        if (h_lookupcmp == h_lookup       &&
            h == map->hashes[i]           &&
            keylen == map->keys[i].keylen &&
           (memcmp(key, map->keys[i].key, keylen) == 0))
        { map->keys[i].keylen=0;
          map->hashlookup[i]=ENTRY_TOMBSTONE;
          return 0; }
    }

    return -1;
}

void *map_lookup(map_t const map, u8 const *key, u64 keylen) {
    u64 h = a5hash128(key, keylen, 0, NULL);
    u8 h_lookup = h >> (64-8) & ENTRY_MASK;

    u64 start = h % map->table_size;
    u64 tsize = map->table_size;
    for (u64 i = start; i != start - 1; i = (i+1)%tsize) {
        u8 h_lookupcmp = map->hashlookup[i];
        if (h_lookupcmp == ENTRY_EMPTY) return NULL;
        if (h_lookupcmp == ENTRY_TOMBSTONE) continue;

        map_key_entry_t k = map->keys[i];
        if (h_lookupcmp == h_lookup &&
            h == map->hashes[i]     &&
            k.keylen == keylen      &&
           (memcmp(key, k.key, keylen) == 0))
        { return map->values[i]; }
    }

    return NULL;
}

#include "map.h"
#include "common.h"
#include "log.h"
#include "smrt_arena.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define ENTRY_TOMBSTONE 0b0000000000000000000000000000000000000000000000000000000000000001
#define ENTRY_MASK      0b1111111111111111111111111111111111111111111111111111111111111110

typedef struct {
    void *key;
    u64 keylen;
} map_key_entry_t;

typedef struct _map_t {
    u64  table_size;
    u64 *hashes;
    map_key_entry_t *keys;
    void **values;
} _map_t;

u32 hash(u8 *input, u64 inputlen) {
    // Make this batched + SIMD eventually
    u32 sum = UINT32_MAX / 2;
    for (u64 i = 0; i < inputlen; i++) {
        sum += input[i];
    }
    return sum;
}

map_t map_create(smrt_arena_t *arena, u64 capacity) {
    map_t map = smrt_arena_push(arena, sizeof(_map_t), true);
    map->hashes = SMRTA_ALLOC_ARRAY(arena, u64, capacity);
    map->keys = SMRTA_ALLOC_ARRAY(arena, map_key_entry_t, capacity);
    map->values = SMRTA_ALLOC_ARRAY(arena, void*, capacity);
    map->table_size=capacity;

    return map;
}

i32 map_insert(map_t map, u8 *key, u64 keylen, void *value) {
    u64 h = hash(key, keylen) & ENTRY_MASK;

    u64 start = h % map->table_size;
    i64 first_tombstone = -1;
    for (u64 i = start; i != start - 1; i = (i+1)%map->table_size) {
        // If didnt find match, insert at first tombstone or current i
        if (map->keys[i].key == NULL) {
            if (first_tombstone != -1) i = first_tombstone;
            map->hashes[i] = h;
            map->keys[i] = (map_key_entry_t){.key=key, .keylen=keylen, };
            map->values[i] = value;
            return 0;
        }

        u64 h_cmp = map->hashes[i];

        // if tombstone, update first tombstone
        if (first_tombstone != -1 && (h_cmp & ENTRY_TOMBSTONE) == ENTRY_TOMBSTONE) {
            first_tombstone = i;
            continue;
        }

        h_cmp &= ENTRY_MASK;
        map_key_entry_t k = map->keys[i];
        if (k.keylen == keylen &&
            h == h_cmp         &&
            memcmp(key, k.key, keylen) != 0)
        {
            map->keys[i].key = key;
            map->values[i] = value;
        }
    }

    return -1;
}

i32 map_delete(map_t map, u8 *key, u64 keylen) {
    u64 h = hash(key, keylen) & ENTRY_MASK;

    u64 start = h % map->table_size;
    for (u64 i = start; i != start - 1; i = (i+1)%map->table_size) {
        if (map->keys[i].key == NULL) {
            return -1;
        }

        u64 h_cmp = map->hashes[i];

        // if tombstone, skip
        if ((h_cmp & ENTRY_TOMBSTONE) == ENTRY_TOMBSTONE) {
            continue;
        }

        h_cmp &= ENTRY_MASK;
        map_key_entry_t k = map->keys[i];
        if (k.keylen == keylen &&
            h == h_cmp         &&
            memcmp(key, k.key, keylen) == 0)
        {
             map->keys[i].keylen = 0;
            *map->hashes &= ENTRY_TOMBSTONE;
        }
    }

    return -1;
}

void *map_lookup(map_t map, u8 *key, u64 keylen) {
    u64 h = hash(key, keylen) & ENTRY_MASK;

    u64 start = h % map->table_size;
    for (u64 i = start; i != start - 1; i = (i+1)%map->table_size) {
        if (map->keys[i].key == NULL) {
            return NULL;
        }

        u64 h_cmp = map->hashes[i];

        // if tombstone, skip
        if ((h_cmp & ENTRY_TOMBSTONE) == ENTRY_TOMBSTONE) {
            continue;
        }

        h_cmp &= ENTRY_MASK;
        map_key_entry_t k = map->keys[i];
        if (k.keylen == keylen &&
            h == h_cmp         &&
            memcmp(key, k.key, keylen) == 0)
        {
            return map->values[i];
        }
    }

    return NULL;
}

void map_print(map_t map) {
    printf("=== MAP ===\n");
    for (u64 i = 0; i < map->table_size; i++) {
        if (map->values[i] || (map->hashes[i] & ENTRY_TOMBSTONE) == 1) {
            printf("%lu %s %lu\n", i, (char*)map->keys[i].key, *(u64*)map->values[i]);
        } else {
            printf("%lu\n", i);
        }
    }
    printf("=== END ===\n");
}

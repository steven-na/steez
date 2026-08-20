#pragma once

#include "common.h"
#include "smrt_arena.h"

// Hashmap structure that stores u8* keys and void* values.
// API: map_create, map_insert, map_delete, map_lookoop
typedef struct _map_t* map_t;

map_t  map_create(smrt_arena_t *arena, u64 table_size);
  i32  map_insert(map_t const map, u8 const *key, u64 keylen, void *value);
  i32  map_delete(map_t const map, u8 const *key, u64 keylen);
 void *map_lookup(map_t const map, u8 const *key, u64 keylen);

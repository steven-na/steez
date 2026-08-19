#pragma once

#include "common.h"
#include "smrt_arena.h"

typedef struct _map_t* map_t;

map_t  map_create(smrt_arena_t *arena, u64 table_size);
  i32  map_insert(map_t map, u8 *key, u64 keylen, void *value);
  i32  map_delete(map_t map, u8 *key, u64 keylen);
 void *map_lookup(map_t map, u8 *key, u64 keylen);

void map_print(map_t map);

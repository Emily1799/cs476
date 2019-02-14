#ifndef __MY_MALLOC_H
#define __MY_MALLOC_H

#include <stdlib.h>

void my_malloc_init(size_t default_size, size_t num_arenas);

void my_malloc_destroy(void);

void* my_malloc(size_t size);


typedef struct arena {
   void *memory;
   size_t size;
   size_t available;
   void* top;
   struct arena* next;
} arena;

arena* generate_single_arena(size_t);
arena* find_arena_space(size_t);

typedef struct {
  arena* arena_top;
  size_t default_size;
} allocator;


#endif /* __MY_MALLOC_H */

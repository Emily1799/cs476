#ifndef __MY_MALLOC_H
#define __MY_MALLOC_H

#include <stdlib.h>
#include <pthread.h>

void my_malloc_init(size_t default_size, size_t num_arenas);

void my_malloc_destroy(void);

void* my_malloc(size_t size);

typedef struct mem_node {
  void* memory;
  struct mem_node *next;
} mem_node;

typedef struct arena {
   void* memory;
   size_t size;
   size_t available;
   void* top;
   pthread_mutex_t a_lock;
   struct arena* next;
} arena;

arena* generate_single_arena(size_t);
arena* find_arena_space(int);
void print_LL(mem_node*);

typedef struct {
  arena* arena_top;
  mem_node* old_memory;
  size_t default_size;
  size_t num_arenas;
} allocator;


#endif /* __MY_MALLOC_H */

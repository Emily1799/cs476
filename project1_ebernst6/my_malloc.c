#include "my_malloc.h"

#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>


#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <string.h>
allocator* info;

void my_malloc_init(size_t default_size, size_t num_arenas) { 
  //initialize global state
  info = (allocator*) malloc(sizeof(allocator));
  info->default_size = default_size;
  //create arenas and make them a linkedlist, by inserting them each at front.
  arena* prev = NULL;
  for(int i =0; i < num_arenas; i++) {
    arena* cur = generate_single_arena(default_size);
    if (cur == NULL) {
      //if malloc failed for some reason, try again.Hopefully shouldn't happen?
      i--;
      continue;
    }
    if(prev != NULL) {
      prev->next = cur;
    }
    prev = cur;
  }
  info->arena_top = prev;
}


//generates a single arena. User must fill in next pointers.
//returns a pointer to the new arena, or NULL on failure to allocate either the arena itself or the memory inside it
arena* generate_single_arena(size_t size) {
  arena* cur = malloc(sizeof(arena));

  if (cur == NULL) {
    return NULL;
  }
  cur->memory = malloc(size);
  if (cur->memory == NULL) {
    //we've failed to allocate the memory in the arena. Free the struct.
    free(cur);
    return NULL;
  }
  cur->size = size;
  cur->available = size;
  cur->top = cur->memory;
  cur->next = NULL;
  return cur;
}

//go through the linked list and delete each arena. First, free the memory inside, then the struct itself. Finally, delete info.
//TODO: set every goddamn lock before starting this, otherwise everything will fuck up.
void my_malloc_destroy(void) {
  arena* prev = NULL;
  arena* cur = info->arena_top;

  while(cur != NULL) {
    if(cur->memory != NULL){
    	free(cur->memory);
    }
    prev = cur;
    cur = cur->next;
    free(prev);
  }
  free(info);

}

//returns an arena that has size bytes available. Returns NULL if there is no such arena
arena* find_arena_space(size_t size) {
  arena* cur = info->arena_top;
  while(cur != NULL) {
    if(cur->available >= size) {
     break;
    }
    else {
      cur = cur->next;
    }
  }
  return cur;
}

void* my_malloc(size_t size) {
  arena* loc = find_arena_space(size);
  void* ret_memory = NULL;
  //if we found an arena with space, so allocate, and return
  if(loc != NULL) {
    ret_memory = loc->top;
    loc->top += size;
    loc->available -= size;
    return ret_memory;
  }
  //if we found no arena with enough space, create a new one
  if(loc == NULL) {
    //if we actually need more than the default gives us, generate an arena twice the size
    if (size > info->default_size) {
      loc = generate_single_arena(size * 2);
    }
    else {
      loc = generate_single_arena(info->default_size);
    }

    //(2 hours of debugging, and a night of sleep later) we're saving this arena, so allocate it on the heap
    arena* perm = malloc(sizeof(arena));
    memcpy(perm, loc, sizeof(arena));
    //gotta free the old one though (that's another hour of debugging...)
    free(loc);

    loc = perm;

    //add our new arena  to info
    loc->next = info->arena_top;
    info->arena_top = loc;

   //actually set aside memory for the user =)
    ret_memory = loc->top;
    loc->top += size;
    loc->available -= size;
    return ret_memory;

  }
  return ret_memory;
}


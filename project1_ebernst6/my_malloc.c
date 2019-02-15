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
  info->num_arenas = num_arenas;

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
  //also free old memory
  mem_node* cur2 = info->old_memory;
  while(cur2 != NULL) {
    free(cur2->memory);
    mem_node* prev2 = cur2;
    cur2 = cur2->next;
    free(prev2);
  }
  free(info);

}

//returns an arena that has size bytes available. Returns NULL if there is no such arena
arena* find_arena_space(int mod) {
  arena* cur = info->arena_top;
  if (info->arena_top == NULL) {
    return NULL;
  }
  if( mod == 0) {
    return info->arena_top;
  }
  for(int i = 0; i < mod; i++) {
    if (cur != NULL) {
      cur = cur->next;
    }
    else {
       cur = info->arena_top;
    }
  }
  return cur;
}

void* my_malloc(size_t size) {
  pthread_t id = pthread_self();
  int mod = id % info->num_arenas;

  arena* loc = find_arena_space(mod);

  void* ret_memory;
  //if we found an arena with space, so allocate, and return
  if(loc->available >size ) {
    ret_memory = loc->top;
    loc->top += size;
    loc->available -= size;
  }
  else { //save the pointer and replace it.
    mem_node* new_ptr = malloc(sizeof(mem_node));
    new_ptr->memory = loc->memory;

    //put old memory onto linked list
    new_ptr->next = info->old_memory;
    info->old_memory = new_ptr;

    //update loc's new information
    loc->memory = malloc(size * 2);
    loc->available = size*2;
    loc->top = loc->memory;
    loc->size = size*2;

    //finally, allocate the space to return
    ret_memory = loc->top;
    loc->top += size;
    loc->available -= size;
  }
  return ret_memory;
}


#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "heap.h"

#define INITIAL_ALLOCATION 65536



static void *heap_start = NULL;
static void *first_free = NULL;


void print_heap(void) {
  printf("heap_start: %p\n", heap_start);
  for (ChunkBoundary *current = (ChunkBoundary*)heap_start;
       current < (ChunkBoundary*)((char*)heap_start + INITIAL_ALLOCATION);
       current = (ChunkBoundary*)((char*)current + (current->size & ~1) + 2 * sizeof(ChunkBoundary))) {
    printf("Chunk: %p\n", current);
    printf("Chunk size: %zu\n", current->size & ~1);
    printf("Chunk allocated: %d\n\n", (int)current->size & 1);
  }
}


void heap_init(void) {
  heap_start = sbrk(INITIAL_ALLOCATION);

  ChunkBoundary *header = (ChunkBoundary*)heap_start;
  header->size = INITIAL_ALLOCATION - 2 * sizeof(ChunkBoundary);  // leave space for header and footer

  ChunkBoundary *footer = (ChunkBoundary*)((char*)heap_start + INITIAL_ALLOCATION - sizeof(ChunkBoundary));
  footer->size = header->size;

  *(void**)((char*)header + sizeof(ChunkBoundary)) = NULL;  // previous free chunk NULL since only 1 chunk
  *(void**)((char*)header + 2 * sizeof(ChunkBoundary)) = NULL;  // next free chunk NULL since first chunk

  first_free = header;
}


void *get_footer(void *header) {
  return (void*)((char*)header + sizeof(ChunkBoundary) + (((ChunkBoundary*)header)->size & ~1));
}

void *heap_malloc(size_t size) {
  size = (size + 7) & ~7;  // 8-bytes align size.
  if (first_free == NULL) return NULL;

  ChunkBoundary *current = first_free;
  // TODO: implement end of chunks check for growing sbrk.
  void **prev_next = NULL;
  while (current->size < size) {
    prev_next = (void **)(current + 1);  // save pointer to previous's next free pointer.
    current = *((ChunkBoundary**)(current + 1));  // skip boundary tag to read next address in unallocated chunk.
  }

  // For now, assume after the loop we found a good chunk.
  void **old_next = (void **)(current + 1);  // save previous next value

  size_t previous_size = current->size;
  current->size = size | 1;  // update chunk size and mark as allocated
  ChunkBoundary *footer = (ChunkBoundary*)get_footer(current);  // generate new footer
  footer->size = current->size;

  ChunkBoundary *next = footer + 1;  // generate next tag
  if (prev_next) *prev_next = (void *)next;  // update previous chunk's next_free pointer
  else first_free = next;  // if NULL, we are at first free. update it.
  *((void**)(next + 1)) = *old_next;  // set next free in new chunk
  next->size = (previous_size - size - 2 * sizeof(ChunkBoundary)) & ~1;  // mark as unallocated (shouldn't be necessary, but idk)
  ChunkBoundary *next_footer = (ChunkBoundary*)get_footer(next);
  next_footer->size = next->size;

  return (void*)(current + 1);  // return DATA pointer of allocated chunk
}

void heap_free(void *p) {
  /* TODO:
   *   - Mark chunk at p unallocated
   *   - Check if previous neighbor unallocated for fusion (shouldn't need any next updates here... I think)
   *   - Check if next neighbor unallocated for fusion (do this one after or previous next update will need to be checked twice I think)
   *     - If case, update previous unallocated chunk's next pointer to p
   *     - Two ways (both are bad imo):
   *       1. traverse unallocated chain until next = p's next
   *       2. go FROM p backward until unallocated
   *   - There shouldn't need to be iterations of this, since if in every free I fuse direct neighbors it should just work out...?
   */
  ChunkBoundary *header = (ChunkBoundary*)((char*)p - sizeof(ChunkBoundary));
  ((ChunkBoundary*)header)->size &= ~1;  // mark p as unallocated

  ChunkBoundary *previous_footer = (ChunkBoundary*)((char*)header - sizeof(ChunkBoundary));
  if ((previous_footer->size & 1) == 0) {
    // TODO: fuse previous neighbor
  }

  ChunkBoundary *next_header = (ChunkBoundary*)((char*)header + (((ChunkBoundary*)header)->size) + 2 * sizeof(ChunkBoundary));
  if ((next_header->size & 1) == 0) {
    // TODO: fuse next neighbor (fun :D)
  }
}
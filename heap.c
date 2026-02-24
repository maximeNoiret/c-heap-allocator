
#include <stdlib.h>
#include <unistd.h>
#include "heap.h"

#define INITIAL_ALLOCATION 65536



static void *heap_start = NULL;
static void *first_free = NULL;


void heap_init(void) {
  heap_start = sbrk(INITIAL_ALLOCATION);

  ChunkBoundary *header = (ChunkBoundary*)heap_start;
  header->size = INITIAL_ALLOCATION - 2 * sizeof(ChunkBoundary);  // leave space for header and footer

  ChunkBoundary *footer = (ChunkBoundary*)((char*)heap_start + INITIAL_ALLOCATION - sizeof(ChunkBoundary));
  footer->size = header->size;

  *(void**)((char*)header + sizeof(ChunkBoundary)) = NULL;  // next free chunk NULL since only 1 chunk

  first_free = header;
}

void *heap_malloc(size_t size) {
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
  ChunkBoundary *footer = (ChunkBoundary*)((char*)current + sizeof(ChunkBoundary) + size);  // generate new footer
  footer->size = current->size;

  ChunkBoundary *next = footer + 1;  // generate next tag
  if (prev_next) *prev_next = (void *)next;  // update previous chunk's next_free pointer
  else first_free = next;  // if NULL, we are at first free
  *((void**)(next + 1)) = *old_next;  // set next free in new chunk
  next->size = (previous_size - size - 2 * sizeof(ChunkBoundary)) & ~1;  // mark as unallocated (shouldn't be necessary, but idk)
  ChunkBoundary *next_footer = (ChunkBoundary*)((char*)next + sizeof(ChunkBoundary) + next->size);
  next_footer->size = next->size;

  if (first_free == current) first_free = next;

  return (void*)(current + 1);
}
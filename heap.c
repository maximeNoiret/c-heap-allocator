#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include "heap.h"

#define INITIAL_ALLOCATION 65536



static void *heap_start = NULL;
static void *first_free = NULL;


void print_heap(void) {
  printf("heap_start: %p\n", heap_start);
  printf("first_free: %p\n", first_free);
  for (ChunkBoundary *current = (ChunkBoundary*)heap_start;
       current < (ChunkBoundary*)((char*)heap_start + INITIAL_ALLOCATION);
       current = (ChunkBoundary*)((char*)current + (current->size & ~1) + 2 * sizeof(ChunkBoundary))) {
    printf("Chunk: %p\n", current);
    printf("Chunk size: %zu\n", current->size & ~1);
    printf("Chunk allocated: %d\n", (int)current->size & 1);
    if ((current->size & 1) == 0) {
      printf("  Previous unallocated: %p\n", *(void**)(current + 1));
      printf("  Next unallocated:     %p\n\n", *(void**)((char*)current + sizeof(ChunkBoundary) + sizeof(void*)));
    } else printf("\n");
  }
}


void heap_init(void) {
  heap_start = sbrk(INITIAL_ALLOCATION);

  ChunkBoundary *header = (ChunkBoundary*)heap_start;
  header->size = INITIAL_ALLOCATION - 2 * sizeof(ChunkBoundary);  // leave space for header and footer

  ChunkBoundary *footer = (ChunkBoundary*)((char*)heap_start + INITIAL_ALLOCATION - sizeof(ChunkBoundary));
  footer->size = header->size;

  *(void**)((char*)header + sizeof(ChunkBoundary)) = NULL;  // previous free chunk NULL since only 1 chunk
  *(void**)((char*)header + sizeof(ChunkBoundary) + sizeof(void*)) = NULL;  // next free chunk NULL since first chunk

  first_free = header;
}

void *get_header(void *footer) {
  return (void*)((char*)footer - sizeof(ChunkBoundary) - (((ChunkBoundary*)footer)->size & ~1));
}

void *get_footer(void *header) {
  return (void*)((char*)header + sizeof(ChunkBoundary) + (((ChunkBoundary*)header)->size & ~1));
}

void *heap_malloc(size_t size) {
  size = (size + 7) & ~7;  // 8-bytes align size.
  if (first_free == NULL) return NULL;

  ChunkBoundary *current = first_free;
  void **prev_next = NULL;
  while ((char*)current < (char*)heap_start + INITIAL_ALLOCATION && current->size < size) {
    prev_next = (void **)(current + 1);  // save pointer to previous's next free pointer.
    current = *((ChunkBoundary**)(current + 1));  // skip boundary tag to read next address in unallocated chunk.
  }
  if ((char*)current >= (char*)heap_start + INITIAL_ALLOCATION) {
    return NULL;  // TODO: expend allocated memory using sbrk.
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
  ChunkBoundary *header = (ChunkBoundary*)((char*)p - sizeof(ChunkBoundary));
  ((ChunkBoundary*)header)->size &= ~1;  // mark p as unallocated (is a waste if previous neighbor is unallocated, who cares it's 1 AND operation.)

  // clear potential garbage data
  *(void**)((char*)header + sizeof(ChunkBoundary)) = NULL;
  *(void**)((char*)header + sizeof(ChunkBoundary) + sizeof(void*)) = NULL;

  // fuse previous neighbor
  ChunkBoundary *previous_footer = (ChunkBoundary*)((char*)header - sizeof(ChunkBoundary));
  if ((void*)previous_footer >= heap_start && (previous_footer->size & 1) == 0) {
    ChunkBoundary *footer = (ChunkBoundary*)get_footer(header);
    footer->size = header->size + previous_footer->size + 2 * sizeof(ChunkBoundary);
    ChunkBoundary *previous_header = (ChunkBoundary*)get_header(previous_footer);
    previous_header->size = footer->size;
    header = previous_header;
  }

  // fuse next neighbor
  ChunkBoundary *next_header = (ChunkBoundary*)((char*)header + (((ChunkBoundary*)header)->size) + 2 * sizeof(ChunkBoundary));
  if ((next_header->size & 1) == 0) {
    ChunkBoundary *prev_free_header = *(ChunkBoundary**)(next_header + 1);
    ChunkBoundary *next_free_header = *(ChunkBoundary**)((char*)next_header + sizeof(ChunkBoundary) + sizeof(void*));

    *(void**)(header + 1) = *(void**)(next_header + 1);
    *(void**)((char*)header + sizeof(ChunkBoundary) + sizeof(void*)) = next_free_header;

    if (prev_free_header)
      *(void**)((char*)prev_free_header + sizeof(ChunkBoundary) + sizeof(void*)) = header;  // update previous unallocated chunk's next pointer.
    if (next_free_header)
      *(void**)((char*)next_free_header + sizeof(ChunkBoundary)) = header;  // update next unallocated chunk's previous pointer.

    header->size += next_header->size + 2 * sizeof(ChunkBoundary);
    ChunkBoundary *footer = (ChunkBoundary*)get_footer(header);
    footer->size = header->size;
  }

  if (first_free > (void*)header) {
    *(void**)(header + 1) = NULL;
    if (*(void**)((char*)header + sizeof(ChunkBoundary) + sizeof(void*)) == NULL)
      *(void**)((char*)header + sizeof(ChunkBoundary) + sizeof(void*)) = first_free;
    *(void**)((char*)first_free + sizeof(ChunkBoundary)) = header;
    first_free = (void*)header;
  }

  // cleanup
  if (*(void**)(header + 1) == header) *(void**)(header + 1) = NULL;
  if (*(void**)((char *)header + sizeof(ChunkBoundary) + sizeof(void*)) == header)
    *(void**)((char *)header + sizeof(ChunkBoundary) + sizeof(void*)) = NULL;
}
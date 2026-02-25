#ifndef HEAP_H_
#define HEAP_H_

typedef struct {
  size_t size;
} ChunkBoundary;


void print_heap(void);

void heap_init(void);

void *heap_malloc(size_t size);

void heap_free(void *p);



#endif // HEAP_H_
#ifndef HEAP_H_
#define HEAP_H_

typedef struct {
  size_t size;
} ChunkBoundary;


void heap_init(void);

void *heap_malloc(size_t size);




#endif // HEAP_H_
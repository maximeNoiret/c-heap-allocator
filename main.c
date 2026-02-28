#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "heap.h"



int main() {
  heap_init();
  
  size_t *ptr1 = heap_malloc(sizeof(size_t));
  *ptr1 = 127;

  int *ptr2 = heap_malloc(sizeof(int));
  *ptr2 = 17;

  char *ptr3 = heap_malloc(14 * sizeof(char));
  memcpy(ptr3, "Hello, World!", 13);
  *(ptr3 + 13) = '\0';

  print_heap();

  printf("ptr1 content: %zu\n", *ptr1);
  printf("ptr2 content: %d\n", *ptr2);
  printf("ptr3 string:  %s\n", ptr3);

  printf("\n\nNow freeing ptr2.\n\n");

  heap_free(ptr2);
  print_heap();

  printf("\n\nNow freeing ptr1.\n\n");

  heap_free(ptr1);
  print_heap();

  printf("\n\nNow allocating 24 bytes. (in ptr1)\n\n");

  ptr1 = heap_malloc(24);  // over allocate just to test 24 bytes :p
  print_heap();

  printf("\n\nNow allocating 3 times 32 bytes. (in ptr2 & new ptr4 & new ptr5) to test 2 unallocated neighbor edge case.\n\n");

  ptr2 = heap_malloc(32);
  size_t *ptr4 = heap_malloc(32);
  size_t *ptr5 = heap_malloc(32);

  printf("Value of ptr4: %p\nValue of ptr5: %p\n\n", ptr4, ptr5);

  print_heap();

  printf("\n\nNow freeing ptr1 and ptr4 to isolate ptr2 between 2 unallocated neighbors.\n\n");

  heap_free(ptr1);
  heap_free(ptr4);

  print_heap();

  printf("\n\nNow freeing ptr3.\n\n");

  heap_free(ptr3);
  print_heap();

  return 0;
}
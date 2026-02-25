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

  printf("\n\nNow freeing ptr3.\n\n");

  heap_free(ptr3);
  print_heap();

  return 0;
}
#include <stdlib.h>

#include "memory.h"

void *reallocate(void *ptr, size_t oldSize, size_t newSize) {
  if (newSize == 0) {
    // That means we have to free it.
    free(ptr);
    return NULL;
  }

  // Else, we rely on <stdlib.h>'s realloc().
  void *result = realloc(ptr, newSize);

  if (result == NULL) {
    // Allocation failed!
    exit(1);
  }

  return result;
}
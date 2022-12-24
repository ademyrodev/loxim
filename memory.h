#ifndef CLOXIM_MEMORY_H
#define CLOXIM_MEMORY_H

#include "common.h"

// Macro to grow the capacity of any array.
#define GROW_CAPACITY(capacity) \
  ((capacity) < 8 ? 8 : (capacity) * 2)
// Set it to 8 if capacity == 0 because 0 * 2 == 0.

// Macro to grow any array.
#define GROW_ARRAY(type, ptr, oldCount, newCount) \
  (type *) reallocate(ptr, sizeof (type) * oldCount, \
  sizeof (type) * newCount)

// Macro to free any array
#define FREE_ARRAY(type, ptr, oldCount) \
  /* Reallocate it to 0 */ \
  reallocate(ptr, sizeof (type) * oldCount, 0)

// Calls our own `reallocate()` function.
// We will be using this function to free,
// reallocate and allocate objects.
void *reallocate(void *, size_t, size_t);

#endif
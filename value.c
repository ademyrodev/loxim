#include <stdio.h>

#include "memory.h"
#include "value.h"

// These functions are very similar to chunk.c functions.

void initValueArray(ValueArray *array) {
  // Fresh, zeroed state.
  array->values = NULL;
  array->capacity = 0;
  array->count = 0;
}

void writeValueArray(ValueArray *array, Value value) {
  if (array->capacity < array->count + 1) {
    // Grow our capacity
    int oldCapacity = array->capacity;
    array->capacity = GROW_CAPACITY(oldCapacity);

    // Now grow the array.
    array->values = GROW_ARRAY(Value, array->values,
                               oldCapacity, array->capacity);

  }

  // Append the value.
  array->values[array->count++] = value;
}

void freeValueArray(ValueArray *array) {
  // Free the array itself.
  FREE_ARRAY(Value, array->values, array->capacity);

  // Zero it out.
  initValueArray(array);
}

void printValue(Value value) {
  switch (value.type) {
    case VAL_NUMBER:  
      printf("%g", AS_NUMBER(value));
      break;

    case VAL_BOOL:
      // Without this ternary operators, Loxim would
      // print either 1 or 0. We don't want that - we want
      // true or false. 
      printf(AS_BOOL(value) ? "true" : "false");
      break;

    case VAL_NIL:
      printf("nil"); 
      break;
  }
}
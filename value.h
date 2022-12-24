#ifndef CLOXIM_VALUE_H
#define CLOXIM_VALUE_H

#include "common.h"

// It's just a double for now. It will become
// more complex later.
typedef double Value;

// Our constant pool.
typedef struct {
  int capacity;
  int count;
  Value *values;
} ValueArray;

// Initializes a value array
void initValueArray(ValueArray *);

// Writes a value to the array
void writeValueArray(ValueArray *, Value);

// Frees the array and zeroes it out.
void freeValueArray(ValueArray *);

// Prints a value.
void printValue(Value);

#endif
#ifndef CLOXIM_VALUE_H
#define CLOXIM_VALUE_H

#include "common.h"

// It's just a double for now. It will become
// more complex later.

typedef enum {
  VAL_BOOL,
  VAL_NIL,
  VAL_NUMBER
} ValueType;

typedef struct {
  ValueType type;

  // Our tagged union.
  union {
    bool boolean;
    double number;
  } as;
} Value;

#define AS_BOOL(value)    ((value).as.boolean)
#define AS_NUMBER(value)  ((value).as.number)

#define BOOL_VAL(value)   ((Value) {VAL_BOOL, {.boolean = value}})
#define NIL_VAL           ((Value) {VAL_NIL, {.number = 0}})
#define NUMBER_VAL(value) ((Value) {VAL_NUMBER, {.number = value}})

#define IS_BOOL(value)    ((value).type == VAL_BOOL)
#define IS_NIL(value)     ((value).type == VAL_NIL)
#define IS_NUMBER(value)  ((value).type == VAL_NUMBER)

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
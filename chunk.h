#ifndef CLOXIM_CHUNK_H
#define CLOXIM_CHUNK_H

#include "common.h"
#include "value.h"

typedef enum {
  OP_CONSTANT,
  OP_CONSTANT_LONG,
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_NEGATE,
  OP_RETURN
} OpCode;

typedef struct {
  int offset;
  int line;
} LineStart;

typedef struct {
  // Current amount of slots in use in
  // the code* array.
  int count;

  // Maximum capacity of the code* array.
  int capacity;

  // All OpCode instructions.
  uint8_t *code;

  // The chunk's constant pool.
  ValueArray constants;

  // Line information - compressed with run-length
  // encoding.
  int lineCount;
  int lineCapacity;
  LineStart *lines;

  // Columns - not compressed because most instructions
  // have a different column.
  int *columns;
} Chunk;

// Initializes a chunk.
void initChunk(Chunk *);

// Writes an instruction to [chunk].
void writeChunk(Chunk *, uint8_t, int, int);

void writeConstant(Chunk *, Value, int, int);

// Adds a constant to the chunk's constant pool.
int addConstant(Chunk *, Value);

// Retrieves an instruction's line.
int getLine(Chunk *, int);

// Frees the chunk and zeroes it out.
void freeChunk(Chunk *);

#endif
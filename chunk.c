#include <stdlib.h>
#include <stdio.h>

#include "chunk.h"
#include "memory.h"

void initChunk(Chunk *chunk) {
  // Fresh, zeroed state of the chunk.
  chunk->count = 0;
  chunk->capacity = 0;
  chunk->code = NULL;
  chunk->lineCount = 0;
  chunk->lineCapacity = 0;
  chunk->lines = NULL;
  chunk->columns = NULL;

  // Initialize the constant pool.
  initValueArray(&chunk->constants);  
}

void writeChunk(Chunk *chunk, uint8_t byte, int line, int column) {
  if (chunk->capacity < chunk->count + 1) {
    // That means we need to grow our array
    int oldCapacity = chunk->capacity;
    
    // Double the capacity
    chunk->capacity = GROW_CAPACITY(oldCapacity);

    // Now grow the array
    chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, 
                            chunk->capacity);

    chunk->columns = GROW_ARRAY(int, chunk->columns, oldCapacity, 
                          chunk->capacity);

  }

  // Append the byte to it.
  chunk->code[chunk->count] = byte;
  chunk->columns[chunk->count++] = column;

  if (chunk->lineCount > 0 && chunk->lines[chunk->lineCount - 1].line == line) {
    // If we are on the same line, we don't have to append a new one to 
    // chunk->lines.
    return;
  }

  // Append to chunk->lines.
  if (chunk->lineCapacity < chunk->lineCount + 1) {
    int oldCapacity = chunk->lineCapacity;
    chunk->lineCapacity = GROW_CAPACITY(oldCapacity);
    chunk->lines = GROW_ARRAY(LineStart, chunk->lines, oldCapacity, 
                              chunk->lineCapacity);
  }

  LineStart *lineStart = &chunk->lines[chunk->lineCount++];
  lineStart->offset = chunk->count - 1;
  lineStart->line = line;
}

void writeConstant(Chunk *chunk, Value value, int line, int column) {
  int index = addConstant(chunk, value);
  if (index < 256) {
    writeChunk(chunk, OP_CONSTANT, line, column);
    writeChunk(chunk, (uint8_t) index, line, column);
  } else {
    writeChunk(chunk, OP_CONSTANT_LONG, line, column);
    writeChunk(chunk, (uint8_t) (index & 0xff), line, column);
    writeChunk(chunk, (uint8_t) ((index >> 8) & 0xff), line, column);
    writeChunk(chunk, (uint8_t) ((index >> 16) & 0xff), line, column);
  }
}

int addConstant(Chunk *chunk, Value value) {
  // Write it to our constant pool.
  writeValueArray(&chunk->constants, value);

  // Return its index in the constant pool.
  return chunk->constants.count - 1;
}

int getLine(Chunk * chunk, int instruction) {
  // Binary search the line
  int start = 0;
  int end = chunk->lineCount - 1;

  while (1) {
    int mid = (start + end) / 2;
    LineStart *line = &chunk->lines[mid];

    if (instruction < line->offset) {
      end = mid - 1;
    } else if (mid == chunk->lineCount - 1 || instruction < chunk->lines[mid + 1]
              .offset) {
      
      return line->line;
    } else {
      start = mid + 1;
    }
  }
}

void freeChunk(Chunk *chunk) {
  // Free the instruction array
  FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);

  // Free our line and column information.
  FREE_ARRAY(LineStart, chunk->lines, chunk->lineCapacity);
  FREE_ARRAY(int, chunk->columns, chunk->capacity);
  
  // Free our constants.
  freeValueArray(&chunk->constants);

  // Zero it out.
  initChunk(chunk);
}
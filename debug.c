#include <stdlib.h>
#include <stdio.h>

#include "debug.h"

void disassembleChunk(Chunk *chunk, char *name) {
  // Disassemble each chunk instruction.
  printf("== %s ==\n", name);

  // Iterate through each instruction and disassemble
  // them.
  for (int offset = 0; offset < chunk->count; 
      offset = disassembleInstruction(chunk, offset));
  
  // Nothing.
}

// disassembleInstruction() helper for simple instructions.
static int simpleInstruction(char *name, int offset) {
  // Just print its name and advance.
  printf("%s\n", name);
  return offset + 1;
}

// disassembleInstruction() helper for constant instructions.
static int constantInstruction(char *name, Chunk *chunk, int offset) {
  // Load the constant.
  uint8_t constant = chunk->code[offset + 1];
  printf("%-16s %4d '", name, constant);
  // Print the constants value around ''
  printValue(chunk->constants.values[constant]);
  
  printf("'\n");

  // Jump two instructions forward because this kind
  // of instruction takes two bytes:
  //
  // instruction  operand
  //    [00]        [1]
  // vvvvvvvvvvvvvvvvvvv
  //      2 bytes

  return offset + 2;
}

static int longConstantInstruction(const char* name, Chunk* chunk,
                                   int offset) {

  uint32_t constant = chunk->code[offset + 1] |
                     (chunk->code[offset + 2] << 8) |
                     (chunk->code[offset + 3] << 16);
  printf("%-16s %4d '", name, constant);
  printValue(chunk->constants.values[constant]);
  printf("'\n");
  return offset + 4;
}

// Disassemble an individual instruction.
int disassembleInstruction(Chunk *chunk, int offset) {
  // The instruction index
  printf("%04d ", offset);
  int line = getLine(chunk, offset);  

  if (offset > 0 && line == getLine(chunk, offset - 1)) {
    // If the current line is the same as the previous one
    printf("    | ");
} else {
    // Print the line number
    printf(" %4d ", line);
  }

  // Example:
  //
  // 0000 1 OP_RETURN
  // 0001 | OP_RETURN
  //      ^-- same line 

  // Print the column
  printf(" %2d ", chunk->columns[offset]);

  uint8_t instruction = chunk->code[offset];
  // Check its type - if it's an OP_RETURN, return this,
  // if its an OP_ADD, return this, etc...
  switch (instruction) {
    case OP_RETURN:
      return simpleInstruction("OP_RETURN", offset);

    case OP_CONSTANT:
      // Since this instruction has an operand, we should
      // handle it properly.
      return constantInstruction("OP_CONSTANT", chunk, offset);

    case OP_CONSTANT_LONG:
      // In case we run out of constant slots.
      return longConstantInstruction("OP_CONSTANT_LONG", chunk, offset);

    case OP_NIL:
      return simpleInstruction("OP_NIL", offset);

    case OP_TRUE:
      return simpleInstruction("OP_TRUE", offset);
      
    case OP_FALSE:
      return simpleInstruction("OP_FALSE", offset);

    case OP_ADD:
      return simpleInstruction("OP_ADD", offset);

    case OP_SUBTRACT:
      return simpleInstruction("OP_SUBTRACT", offset);

    case OP_MULTIPLY:
      return simpleInstruction("OP_MULTIPLY", offset);

    case OP_DIVIDE:
      return simpleInstruction("OP_DIVIDE", offset);

    case OP_NEGATE:
      return simpleInstruction("OP_NEGATE", offset);
    
    default:
      printf("Unknown OPCODE %d\n", instruction);
      // Advance one instruction forward.
      return offset + 1;
  }
}
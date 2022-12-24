#ifndef CLOXIM_VM_H
#define CLOXIM_VM_H

#include "chunk.h"
#include "value.h"

#define STACK_MAX 256

// Our virtual machine - the thing that will
// execute code. Beware!
typedef struct {
  Chunk *chunk;
  
  // Instruction ptr.
  uint8_t *ip;

  // The VM's stack.
  Value stack[STACK_MAX];
  Value *stackTop;
} VM;

// If execution was successful or not.
typedef enum {
  INTERPRET_OK,
  INTERPRET_COMPILE_ERROR,
  INTERPRET_RUNTIME_ERROR
} InterpretResult;

// Since our VM is a global variable, we don't
// need to worry about parameters.

// Initializes the VM.
void initVM();

// Winds down the VM.
void freeVM();

// Runs a chunk of bytecode.
InterpretResult interpret(char *source);

// Stack functions.
void push(Value value);

Value pop();

#endif
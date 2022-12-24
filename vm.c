#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "vm.h"
#include "debug.h"

// Our global VM.
VM vm;

// Helper functions.
static void resetStack() {
  // Make the stack top point to the first
  // stack slot.
  vm.stackTop = vm.stack;
}

// Stack functions.
void push(Value value) {
  *vm.stackTop = value;

  // Advance the stackTop pointer.
  vm.stackTop++;
}

Value pop() {
  vm.stackTop--;
  return *vm.stackTop;
}

void initVM() {
  resetStack();
}

void freeVM() {

}

static InterpretResult run() {
#define READ_BYTE()     (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(op) \
  do { \
    double b = pop(); \
    double a = pop(); \
    push(a op b); \
  } while (false)

  while (1) {
#ifdef DEBUG_TRACE_EXECUTION
    // Print the contents of the stack:
    printf("      ");
    for (Value *slot = vm.stack; slot < vm.stackTop; slot++) {
      printf("[ ");
      printValue(*slot);
      printf(" ]");
    }

    printf("\n");
    disassembleInstruction(vm.chunk, (int) 
                          (vm.ip - vm.chunk->code));
#endif
    // Visit each instruction.
    uint8_t instruction;
    switch (instruction = READ_BYTE()) {
      case OP_CONSTANT: {
        // The operand:
        Value constant = READ_CONSTANT();

        // Push it onto the stack.
        push(constant);

        printf("\n");
        break;
      }

      case OP_NEGATE:
        // Negate the top of the stack and return
        // it.
        // Offsetting stackTop by -1 because it always
        // points to the next slot to be occupied,
        // if that makes sense.
        vm.stackTop[-1] = -vm.stackTop[-1];
        break;

      // Binary operations.
      case OP_ADD:
        BINARY_OP(+);
        break;

      case OP_SUBTRACT:
        BINARY_OP(-);
        break;

      case OP_MULTIPLY:
        BINARY_OP(*);
        break;

      case OP_DIVIDE:
        BINARY_OP(/);
        break;

      case OP_RETURN:
        // Prints the top of the stack
        // (for now, of course).
        printValue(pop());
        printf("\n");
        return INTERPRET_OK;
    }
  }

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

InterpretResult interpret(char *source) {
  Chunk chunk;
  initChunk(&chunk);

  if (!compile(source, &chunk)) {
    // Error.
    freeChunk(&chunk);
    return INTERPRET_COMPILE_ERROR;
  }

  vm.chunk = &chunk;
  vm.ip = vm.chunk->code;

  InterpretResult result = run();

  freeChunk(&chunk);
  return result;
}
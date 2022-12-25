#include <stdarg.h>
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

static void runtimeError(char *format, ...) {
  // Using variadic arguments to format *format.
  // Make the list stuff
  va_list args;
  va_start(args, format);
  
  // Print the format string
  fprintf(stderr, "Runtime error: ");
  vfprintf(stderr, format, args);

  // Get the line and column
  size_t instruction = vm.ip - vm.chunk->code - 1;
  int lineNumber = getLine(vm.chunk, instruction);
  int column = vm.chunk->columns[instruction];

  // Print the line info
  fprintf(stderr, "\nLine %d, column %d", lineNumber, column);
  fputs("\n", stderr);

  // Retrieve the line where the error occured
  // Note: this function is defined in compiler.c
  char *line = getOffendingLine(lineNumber);

  // Print it
  fprintf(stderr, "%5d | %s\n", lineNumber, line);
  // Show the caret (^-- Here.)
  fprintf(stderr, "%*s", 7 + column, "");
  //                     ^^^^^^^^^^-- distance - amount of spaces.

  // Since we added enough spaces, we can now just print the ^-- Here. message.
  fprintf(stderr, "^-- Here.\n");
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

Value peek(int distance) {
  return vm.stackTop[-1 - distance];
}

void initVM() {
  resetStack();
}

void freeVM() {

}

static InterpretResult run() {
#define READ_BYTE()     (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(valueType, op) \
  do { \
    if (!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
      runtimeError("Operands must be numbers."); \
      return INTERPRET_COMPILE_ERROR; \
    } \
    double b = AS_NUMBER(pop()); \
    double a = AS_NUMBER(pop()); \
    push(valueType(a op b)); \
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

      // Dedicated constant instructions.
      case OP_NIL:
        push(NIL_VAL);
        break;

      case OP_TRUE:
        push(BOOL_VAL(true));
        break;

      case OP_FALSE:
        push(BOOL_VAL(false));
        break;

      case OP_NEGATE:
        // Negate the top of the stack and return
        // it.
        if (!IS_NUMBER(peek(0))) {
          runtimeError("Operand must be a number.");
          return INTERPRET_RUNTIME_ERROR;
        }

        // Offsetting stackTop by -1 because it always
        // points to the next slot to be occupied,
        // if that makes sense.
        vm.stackTop[-1] = NUMBER_VAL(-AS_NUMBER(vm.stackTop[-1]));
        break;

      // Binary operations.
      case OP_ADD:
        BINARY_OP(NUMBER_VAL, +);
        break;

      case OP_SUBTRACT:
        BINARY_OP(NUMBER_VAL, -);
        break;

      case OP_MULTIPLY:
        BINARY_OP(NUMBER_VAL, *);
        break;

      case OP_DIVIDE:
        BINARY_OP(NUMBER_VAL, /);
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
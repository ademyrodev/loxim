#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// My own implementation of Lox.

#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

static void repl() {
  // Will improve REPL later.
  char line[1024];

  while (1) {
    printf("> ");

    // Read input.
    if (!fgets(line, sizeof (line), stdin)) {
      printf("\n");
      break;
    }

    // Pass it to our VM.
    interpret(line);
  }
}

static char *readFile(char *path) {
  // Open the file
  FILE *file = fopen(path, "rb");

  if (file == NULL) {
    fprintf(stderr, "Could not open \"%s\". Make sure you're in the correct"
            " directory.\n", path);
    
    // No need to close it - it is already NULL.
    exit(74);
  }

  // Calculate its size.
  fseek(file, 0L, SEEK_END);
  size_t fileSize = ftell(file);
  rewind(file);

  // Read the file
  char *buf = malloc(fileSize + 1); // +1 for \0

  if (buf == NULL) {
    fprintf(stderr, "Not enough memory to read \"%s\". "
                    "(File size %zu bytes + 1)\n", path, fileSize);
    
    exit(74);
  }

  size_t bytesRead = fread(buf, sizeof (char), fileSize, file);
  
  if (bytesRead < fileSize) {
    fprintf(stderr, "Failed to read \"%s\". This issue is unlikely.\n", path);
    exit(74);
  }

  buf[bytesRead] = '\0';

  // Return it.
  fclose(file);
  return buf;
}

static void runFile(char *path) {
  // Read the contents of the file.
  char *source = readFile(path);

  // Interpret the source code.
  InterpretResult result = interpret(source);

  free(source);

  if (result == INTERPRET_COMPILE_ERROR)
    exit(65);

  if (result == INTERPRET_RUNTIME_ERROR)
    exit(70);
}

int main(int argc, char **argv) {
  initVM();

  if (argc == 1) {
    // Read input, Evaluate, Print, Loop
    repl();
  } else if (argc == 2) {
    runFile(argv[1]);
  } else {
    fprintf(stderr, "Usage: loxm [path]\n");
    exit(64);
  }

  freeVM();
  return 0;
}
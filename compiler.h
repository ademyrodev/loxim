#ifndef CLOXIM_COMPILER_H
#define CLOXIM_COMPILER_H

#include "vm.h"

// Useful for runtimeError() in VM.
char *getOffendingLine(int);

// Compiles a stream of characters.
bool compile(char *, Chunk *);

#endif
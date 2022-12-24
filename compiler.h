#ifndef CLOXIM_COMPILER_H
#define CLOXIM_COMPILER_H

#include "vm.h"

// Compiles a stream of characters.
bool compile(char *, Chunk *);

#endif
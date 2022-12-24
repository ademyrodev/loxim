#ifndef CLOXIM_DEBUG_H
#define CLOXIM_DEBUG_H

#include "chunk.h"

// Users won't really need this module, but we might need
// it to debug stuff.

void disassembleChunk(Chunk *, char *);

// Disassembles a single instruction.
int disassembleInstruction(Chunk *, int);

#endif
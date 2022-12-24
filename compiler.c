#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"

// Like a ParseState in other implementations.
typedef struct {
  char *source;
  Token current;
  Token previous;
} Parser;

Parser parser;

// Errors.

static char *getLine(int line) {
  // I looked for a better way to accomplish that and
  // that's the only way I could find, so yes, it's 
  // O(n).
  
  char *s = NULL;
  size_t lineLen = 0;
  int ln = 0;
  int i = 0;

  // This is extreeeemely slow.
  while (parser.source[i] != '\0') {
    if (ln == line) {
      // Since ln is zero indexed and our line index
      // passed to this function is 1 indexed, that means
      // we have to traceback to the beggining of the line.
      // (I dedided to do this to avoid nested loops)

      s = malloc(lineLen + 1); // Of course, +1 for '\0'
      
      // Copy stuff before the previous \n to s
      memcpy(s, parser.source - lineLen, lineLen);

      s[lineLen] = '\0';

      return s;
    }

    // Not using an if statement here because we don't have to!
    lineLen++;
    i++;
    ln += parser.source[i] == '\n';
  }
}

static void errorAt(Token *token, char *message) {
  fprintf(stderr, "Error: %s\n");

  if (token->type == TOKEN_EOF) {
    fprintf(stderr, "At end\n");
  } else if (token->type == TOKEN_ERROR) {
    // Nothing.
  } else {
    // This prints the token's lexeme.
    fprintf(stderr, "At '%.*s'\n", token->length, token->start);
  }

  int lineNumber = token->line;
  char *line = getLine(lineNumber);
  
  // Count how many digits there are in lineNumber because 
  // we will want to space stuff properly:
  //
  //     15 | function(first, second,);
  //                                ^-- Here.   
  int digits = log10(lineNumber)+1;

  fprintf(stderr, "    %d | %s\n", lineNumber, line);
  fprintf(stderr, "%*s", 4 + digits + token->column, "");
  //                     ^^^^^^^^^^^^^^^^^^^^^^^^^^-- distance - amount of spaces.

  fprintf(stderr, "^-- Here.\n");
}

static void error(char *message) {
  errorAt(&parser.previous, message);
}

static void errorAtCurrent(char *message) {
  errorAt(&parser.current, message);
}

static void advance() {
  parser.previous = parser.current;

  while (1) {
    parser.current = scanToken();
    if (parser.current.type != TOKEN_ERROR)
      break;

    // else
    errorAtCurrent(parser.current.start);
  }
}

bool compile(char *source, Chunk *chunk) {
  // We won't build the compiler - yet.
  initScanner(source);
  parser.source = source;
  advance();
  expression();
  consume(TOKEN_EOF, "Expected end of expression.");
}
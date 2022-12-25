#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "common.h"
#include "compiler.h"
#include "scanner.h"

#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

// Like a ParseState in other implementations.
typedef struct {
  char *source;
  Token current;
  Token previous;
  bool hadError;
  bool panicMode; // Error stuff.
} Parser;

Parser parser;
Chunk *compilingChunk;

static Chunk *currentChunk() {
  return compilingChunk;
}

// Errors.

// This method is not static because the VM might want to use it
char *getOffendingLine(int line) {
  // I looked for a better way to accomplish that and
  // that's the only way I could find, so yes, it's 
  // O(n).
  
  char *s = NULL;
  size_t lineLen = 0;
  int ln = 1;

  // This is extreeeemely slow.
  for (int i = 0; parser.source[i] != '\0'; i++) {
    // I'm sorry, this thing has nested loops.
    if (ln == line) {
      // We are on the correct line. Now we must
      // retrieve it.

      // Calculate the length of the line.
      while (parser.source[i] != '\n' || parser.source[i] == '\0') {
        i++;
        lineLen++;
      }

      // Allocate space for [s]. +1 for the null terminator.
      s = malloc(lineLen + 1);

      // "Paste" the line into s.
      memcpy(s, parser.source + (i - lineLen), lineLen);
      s[lineLen] = '\0';

      // Retrieve it.
      return s;
    }

    ln += parser.source[i] == '\n';
  }

  return NULL;
}

// source = '1 + 1\0'
// s = NULL
// lineLen = 0
// ln = 1

static void errorAt(Token *token, char *message) {
  // Avoid flooding errors
  if (parser.panicMode)
    return;

  int lineNumber = token->line;
  fprintf(stderr, "Error: %s\nLine %d, ", message, lineNumber);

  if (token->type == TOKEN_EOF) {
    fprintf(stderr, "at end of file\n\n");
  } else if (token->type == TOKEN_ERROR) {
    // Nothing.
  } else {
    // This prints the token's lexeme.
    fprintf(stderr, "at '%.*s'\n\n", token->length, token->start);
  }

  char *line = getOffendingLine(lineNumber);

  if (line == NULL) {
    fprintf(stderr, "Line is NULL.\n");
    return;
  }
  
  // Count how many digits there are in lineNumber because 
  // we will want to space stuff properly:
  //
  //     15 | function(first, second,);
  //                                ^-- Here.

  fprintf(stderr, "%5d | %s\n", lineNumber, line);

  // This little extra '2' is the size of the separator between the line number
  // and the line. (" | ") (5 + 2 = 7)
  fprintf(stderr, "%*s", 7 + token->column, "");
  //                     ^^^^^^^^^^^^^^^^^-- distance - amount of spaces.

  // Since we added enough spaces, we can now just print the ^-- Here. message.
  fprintf(stderr, "^-- Here.\n");

  free(line);

  parser.hadError = true;
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

static void consume(TokenType type, char *errorMessage) {
  if (parser.current.type == type) {
    advance();
    return;
  }

  // Not doing
  // return parser.current.type == type;
  // because we want to advance if that condition is true.
  errorAtCurrent(errorMessage);
}

static void emitByte(uint8_t byte, int col) {
  // We could've used parser.previous.column instead of requiring
  // a 'col' parameter, but take a look at this expression:
  //
  // print 1 + 1;
  // 
  // The bytecode sequence generated is:
  // OP_CONST 1
  // OP_CONST 1
  // OP_ADD
  // OP_PRINT
  // Since we emit ADD after the operands, it *might* be on
  // the wrong col.
  writeChunk(currentChunk(), byte, parser.previous.line, col);
}

static void emitBytes(uint8_t byte1, uint8_t byte2, int col) {
  // For OPCODES with operands.
  emitByte(byte1, col);
  emitByte(byte2, col);
}

static void emitReturn(int col) {
  emitByte(OP_RETURN, col);
}

static void emitConstant(Value value, int col) {
  writeConstant(currentChunk(), value, parser.previous.line, col);
}

static void endCompiler(int col) {
  emitReturn(col);
#ifdef DEBUG_PRINT_CODE
  if (!parser.hadError)
    disassembleChunk(currentChunk(), "code");
#endif
}

static void number() {
  double value = strtod(parser.previous.start, NULL);
  emitConstant(NUMBER_VAL(value), parser.previous.column);
}

// Recursive descent parsing.
// Let's forward declare everything since they're
// recursive:
static void expression();

static void term();

static void factor();

static void literal();

// To group expressions around parentheses
static void grouping();

static void unary();

// Let's get down to business:
static void expression() {
  term();

  while (parser.current.type == TOKEN_PLUS || parser.current.type == TOKEN_MINUS) {
    Token operator = parser.current;
    advance();
    term();
    emitByte(operator.type == TOKEN_PLUS ? OP_ADD : OP_SUBTRACT, operator.column);
  }
}

static void term() {
  factor();

  while (parser.current.type == TOKEN_STAR || parser.current.type == TOKEN_SLASH) {
    Token operator = parser.current;
    advance();
    factor();
    emitByte(operator.type == TOKEN_STAR ? OP_MULTIPLY : OP_DIVIDE, operator.column);
  }
}

static void factor() {
  // Keeping track of the column
  int col = parser.current.column;
  Token token = parser.current;

  // This if statement will be moved to another function later.
  // (its name will be literal())
  if (token.type == TOKEN_NUMBER) {
    advance();
    number();
    return;
  }

  if (token.type == TOKEN_LEFT_PAREN) {
    advance();
    grouping();
    return;
  }
  
  if (token.type == TOKEN_BANG || token.type == TOKEN_MINUS || token.type == TOKEN_PLUS) {
    advance();
    unary();
    return;
  }

  // Nothing matches - must be a literal.
  literal();
}

static void literal() {
  switch (parser.current.type) {
    case TOKEN_TRUE:
      emitByte(OP_TRUE, parser.current.column);
      advance();
      break;

    case TOKEN_FALSE:
      emitByte(OP_FALSE, parser.current.column);
      advance();
      break;

    case TOKEN_NIL:
      emitByte(OP_NIL, parser.current.column);
      advance();
      break;

    default:
      // TODO: move numbers to this function.
      // None of the cases match - must be a syntax error.
      // The compiler will find himself in here if he has to deal
      // with an expression such as: "1 + "
      errorAtCurrent("Expected an expression.");
      return;
  }
}

static void grouping() {
  expression();
  consume(TOKEN_RIGHT_PAREN, "Expected ')' after expression.");
}

static void unary() {
  Token operator = parser.previous;

  // Compile the operand first.
  factor();

  switch (operator.type) {
    case TOKEN_MINUS: emitByte(OP_NEGATE, operator.column);
    default: return;
  }
}

bool compile(char *source, Chunk *chunk) {
  // We won't build the compiler - yet.
  initScanner(source);
  compilingChunk = chunk;

  parser.source = source;
  parser.hadError = false;
  parser.panicMode = false;

  advance();
  expression();
  consume(TOKEN_EOF, "Expected end of expression.");
  endCompiler(parser.previous.column);

  // compile() should return false if an error occured.
  return !parser.hadError;
}
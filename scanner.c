#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

// Kind of like a Scanner state struct.
typedef struct {
  // Current lexeme
  char *start;
  char *current;

  // Position
  int line;
  int startCol;
  int currentCol;

  // Some cache values (if I can call them cache values)
  bool isInInterpolation;
} Scanner;

Scanner scanner;

void initScanner(char *source) {
  scanner.start = source;
  scanner.current = source;

  scanner.line = 1;
  scanner.startCol = 1;
  scanner.currentCol = 1;
}

static bool isAlpha(char c) {
  return (c >= 'a' && c <= 'z') ||
         (c >= 'A' && c <= 'Z') ||
          c == '_';
}

static bool isDigit(char c) {
  // It's really great that characters
  // are integers in C.
  return c >= '0' && c <= '9';
}

static bool isAtEnd() {
  return *scanner.current == '\0';
}

static Token makeToken(TokenType type) {
  Token token;
  token.type = type;
  token.start = scanner.start;

  // Calculate the distance between two pointers
  token.length = (int) (scanner.current - scanner.start);

  token.line = scanner.line;
  token.column = scanner.startCol;
  
  return token;
}

// For errors - the compiler will take
// care of those.
static Token errorToken(char *message) {
  Token token;
  token.type = TOKEN_ERROR;
  token.start = message;
  token.length = (int) strlen(message);
  token.line = scanner.line;
  token.column = scanner.startCol;
  return token;
}

static char advance() {
  // In addition of advancing, it also
  // returns the previous char after advancing.
  scanner.current++;
  scanner.currentCol++;
  return scanner.current[-1];
}

static char peek() {
  return *scanner.current;
}

static char peekNext() {
  if (isAtEnd()) 
    return '\0';

  return scanner.current[1];
}

static bool match(char expected) {
  if (isAtEnd())
    return false;

  // Not doing
  // return *scanner.current == expected
  // because we want to advance if 
  // *scanner.current == expected.

  if (*scanner.current != expected)
    return false;

  scanner.current++;
  return true;
}

static void skipWhitespace() {
  while (1) {
    char c = peek();
    switch (c) {
      case '\n':
        scanner.line++;
        scanner.currentCol = 0;
        // Not breaking - we are falling through!
      case ' ':
      case '\r':
      case '\t':
        advance();
        break;

      case '/':
        if (peekNext() == '/') {
          // Comment!
          while (peek() != '\n' && !isAtEnd()) 
            advance();

        } else
          return;

        break;

      default:
        return;
    }
  }
}

static TokenType checkKeyword(int start, int length, char* rest, 
                             TokenType type) {

  // Just rewording the condition:
  // If the distance between scanner.current and scanner.start
  // is the same as start + length and scanner.start offset
  // to the right by start is the same as rest and has the
  // length of length:
  if (scanner.current - scanner.start == start + length &&
      memcmp(scanner.start + start, rest, length) == 0) {

    return type;
  }

  return TOKEN_IDENTIFIER;
}

static TokenType identifierType() {
  switch (*scanner.start) {
    case 'a': return checkKeyword(1, 2, "nd", TOKEN_AND);
    case 'c': 
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'a':
            return checkKeyword(2, 2, "se", TOKEN_CASE);

          case 'l':
            // Don't remove the 'l'. My code must not be 
            // inappropriate.
            return checkKeyword(1, 4, "lass", TOKEN_CLASS);
        }
      }
      break;

    case 'd': return checkKeyword(1, 6, "efault", TOKEN_DEFAULT);
    case 'e': return checkKeyword(1, 3, "lse", TOKEN_ELSE);
    case 'f':
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'a': return checkKeyword(2, 3, "lse", TOKEN_FALSE);
          case 'o': return checkKeyword(2, 1, "r", TOKEN_FOR);
          case 'u': return checkKeyword(2, 1, "n", TOKEN_FUN);
        }
      }
      break;

    case 't':
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'h': return checkKeyword(2, 2, "is", TOKEN_THIS);
          case 'r': return checkKeyword(2, 2, "ue", TOKEN_TRUE);
        }
      }
      break;

    case 'i': 
      if (scanner.current - scanner.start == 2) {
        switch (scanner.start[1]) {
          case 'f':
            // checkKeyword(2, 0, "", TOKEN_IF)
            // would be weird, wouldn't it?
            return checkKeyword(1, 1, "f", TOKEN_IF);

          case 'n':
            return checkKeyword(1, 1, "n", TOKEN_IN);
        }
      } else if (scanner.current - scanner.start > 1)
        return checkKeyword(1, 9, "nstanceof", TOKEN_INSTANCEOF);

      break;

    case 'n': 
      if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'i':
            return checkKeyword(2, 1, "l", TOKEN_NIL);

          case 'm':
            return checkKeyword(2, 2, "ut", TOKEN_NMUT);
        }
      }
      break;

    case 'o': return checkKeyword(1, 1, "r", TOKEN_OR);
    case 'p': return checkKeyword(1, 4, "rint", TOKEN_PRINT);
    case 'r': return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
    case 's':       
    if (scanner.current - scanner.start > 1) {
        switch (scanner.start[1]) {
          case 'u':
            return checkKeyword(2, 3, "per", TOKEN_SUPER);

          case 'w':
            return checkKeyword(2, 4, "itch", TOKEN_SWITCH);
        }
      }
      break;

    case 'v': return checkKeyword(1, 2, "ar", TOKEN_VAR);
    case 'w': return checkKeyword(1, 4, "hile", TOKEN_WHILE);
  }

  return TOKEN_IDENTIFIER;
}

static Token identifier() {
  while (isAlpha(peek()) || isDigit(peek()))
    advance();

  // identifierType() will check for us if 
  // we are on a keyword or an identifier.
  return makeToken(identifierType());
}

static Token number() {
  while (isDigit(peek()))
    advance();

  // Is there a fractional part?
  if (peek() == '.' && isDigit(peekNext())) {
    // Consume the '.'.
    advance();

    while (isDigit(peek()))
      advance();
  }

  return makeToken(TOKEN_NUMBER);
}

static Token string() {
  // We never know: it can be a string interpolation
  // or a simple string.
  TokenType type = TOKEN_STRING;
  while (peek() != '"' && !isAtEnd()) {
    // Lox allows multi-line strings in the book,
    // so I thought about handling these in Loxim too.
    // After all, Loxim is just a superset of Lox.
    // (Yes, any Lox code is valid in Loxim, except for)
    // (a few edge cases (try declaring a local and))
    // ((not using it))
    if (peek() == '\n')
      scanner.line++;

    // String interpolation.
    if (peek() == '$') {
      if (peekNext() != '{')
        return errorToken("Expected '{' after '$' in string interpolation.");
      
      // Consume it inside.
      advance();
      // Set it to true - for later tokens.
      scanner.isInInterpolation = true;
      type = TOKEN_STRING_INTERPOLATION;
      break;
    }

    advance();
  }

  if (isAtEnd())
    return errorToken("Unterminated string.");

  // Advance past the closing quote.
  advance();
  return makeToken(type);
}

Token scanToken() {
  skipWhitespace();
  // Synchronize stuff.
  scanner.start = scanner.current;
  scanner.startCol = scanner.currentCol;

  if (isAtEnd())
    return makeToken(TOKEN_EOF);

  char c = advance();
  if (isAlpha(c))
    return identifier();

  if (isDigit(c))
    return number();

  // Other tokens.
  switch (c) {
    case '(': return makeToken(TOKEN_LEFT_PAREN);
    case ')': return makeToken(TOKEN_RIGHT_PAREN);
    case '[': return makeToken(TOKEN_LEFT_BRACKET);
    case ']': return makeToken(TOKEN_RIGHT_BRACKET);
    case '{': return makeToken(TOKEN_LEFT_BRACE);
    case '}': 
      // If we are on an interpolated expression, 
      // this is the end of the interpolated expression.
      // What's left is the rest of the "parent" string.
      advance();

      if (scanner.isInInterpolation) {
        scanner.isInInterpolation = false;
        return string();
      }

      // else
      return makeToken(TOKEN_RIGHT_BRACE);

    case ';': return makeToken(TOKEN_SEMICOLON);
    case ',': return makeToken(TOKEN_COMMA);
    case '.': return makeToken(TOKEN_DOT);
    case '-': return makeToken(TOKEN_MINUS);
    case '+': return makeToken(TOKEN_PLUS);
    case '/': return makeToken(TOKEN_SLASH);
    case '*': return makeToken(TOKEN_STAR);
    // Multiple char tokens.
    case '!':
      return makeToken(
          match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
    case '=':
      return makeToken(
          match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
    case '<':
      return makeToken(
          match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
    case '>':
      return makeToken(
          match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);

    case '"': return string();
  }

  return errorToken("Unexpected character.");  
}
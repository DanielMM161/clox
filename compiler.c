#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "scanner/scanner.h"

typedef struct {
    Token previous;
    Token current;
    bool hadError;
    bool panicMode;
} Parser;

Parser parser;

static void errorAt(Token *token, const char *message) {
    // Print where the error occurred
    fprintf(stderr, "[line %d] Error", token->line);

    
    if (token->type == TOKEN_EOF) {
        fprintf(stderr, " at end");
    } else if (token->type == TOKEN_ERROR) {
        // nothing
    } else {
        fprintf(stderr, " at '%.*s'", token->length, token->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser.hadError = true;
}

/**
 * Error at the current token
 */
static void errorAtCurrent(const char *message) {
  // pull the location out of the current token
  // in order to tell the user where the error occurred and forwardit to
  // errorAt()
  errorAt(&parser.current, message);
}

static void advance() {
  // take old current token and store it in previous field
  // this will help later to get the lexeme after we watch the token
  parser.previous = parser.current;

  for (;;) {
    // ask the scanner for the next token and stores it for later use
    parser.current = scanToken();
    if (parser.current.type != TOKEN_ERROR)
      break;

    errorAtCurrent(parser.current.start);
  }
}


/**
    Error at the previous token
*/
static void error(const char *message) { 
    errorAt(&parser.previous, message); 
}

bool compile(const char *source, Chunk *chunk) {
  initScanner(source);
  // 'primes the pump' on the scanner
  advance();
  expression();
  consume(TOKEN_EOF, "Expect end of expression.");
  return !parser.hadError;
}



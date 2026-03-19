#include <stdio.h>

#include "common.h"
#include "compiler.h"
#include "scanner/scanner.h"

typedef struct {
    Token previous;
    Token current;
} Parser;

Parser parser;

bool compile(const char *source, Chunk *chunk) {
  initScanner(source);
  // 'primes the pump' on the scanner
  advance();
  expression();
  consume(TOKEN_EOF, "Expect end of expression.");
  return true;
}

static void advance() {
    // take old current token and store it in previous field
    // this will help later to get the lexeme after we watch the token
    parser.previous = parser.current;

    for(;;) {
        // ask the scanner for the next token and stores it for later use
        parser.current = scanToken();
        if(parser.current.type != TOKEN_ERROR) break;

        errorAtCurrent(parser.current.start);        
    }
}
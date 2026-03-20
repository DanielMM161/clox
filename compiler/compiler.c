#include <stdio.h>
#include <stdlib.h>
#include "../scanner/scanner.h"
#include "compiler.h"

typedef struct {
    Token previous;
    Token current;
    bool hadError;
    bool panicMode;
} Parser;

Parser parser;

static void errorAt(Token *token, const char *message) {
    parser.panicMode = true;

    // Print where the error occurred
    fprintf(stderr, "[line %d] Error", token->line);

    
    if (token->type == TOKEN_EOF) {
        fprintf(stderr, ".at end");
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
  // in order to tell the user where the error occurred and forward it to errorAt()
  errorAt(&parser.current, message);
}

/**
 * Read tokens and reports the errors until hit a non error one or reach the end
 */
static void advance() {
    // take old current token and store it in previous field
    // this will help later to get the lexeme after we watch the token
    parser.previous = parser.current;

    for (;;) {
        // ask the scanner for the next token and stores it for later use
        // scanner doesn't report lexical errors. it create 'error tokens' and leaves it up to the parser to report them
        // in that way the rest of the parses only sees valid tokens
        parser.current = scanToken();
        if (parser.current.type != TOKEN_ERROR)
            break;

        errorAtCurrent(parser.current.start);
    }
}

/**
 * Similar to advance() in that it read the next token but validate that token has an expected type if not report error
 */
static void consume(TokenType type, const char* message) {
    if(parser.current.type == type) {
        advance();
        return;
    }

    errorAtCurrent(message);
}


/**
 * rror at the location of the token that was consumed
 */
static void error(const char *message) { 
    errorAt(&parser.previous, message); 
}

bool compile(const char *source, Chunk *chunk) {
    initScanner(source);
    
    parser.hadError = false;
    parser.panicMode = false;


    // 'primes the pump' on the scanner
    advance();
    expression();
    consume(TOKEN_EOF, "Expect end of expression.");
    return !parser.hadError;
}



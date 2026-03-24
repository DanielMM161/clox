#include <cstdint>
#include <stdint.h>
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

typedef enum {
 PREC_NONE,
 PREC_ASSIGNMENT, // =
 PREC_OR, // or
 PREC_AND, // and
 PREC_EQUALITY, // == !=
 PREC_COMPARISON, // < > <= >=
 PREC_TERM, // + -
 PREC_FACTOR, // * /
 PREC_UNARY, // ! -
 PREC_CALL, // . ()
 PREC_PRIMARY
} Precedence;

Chunk* compilingChunk;
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

// Indirection layer so other parts of the compiler don't access compilingChunk directly
static Chunk* currentChunk() {
    return compilingChunk;
}


static void emitByte(uint8_t byte) {
    // After we parse and understand a piece of the user's program
    // the next step is to translate that to a series of bytecode instructions
    writeChunk(currentChunk(), byte, parser.previous.line);
}

// Convenience for instructions that take a one-byte operand (e.g. OP_CONSTANT + index)
static void emitBytes(uint8_t byte1, uint8_t byte2) {
    emitByte(byte1);
    emitByte(byte2);
}

static void emitReturn() {
    // every function must end with a return instruction
    emitByte(OP_RETURN);
}

static void endCompiler() {
    // seal the chunk when we're done compiling
    emitReturn();
}

/**
 * rror at the location of the token that was consumed
 */
static void error(const char *message) { 
    errorAt(&parser.previous, message); 
}

static void parsePrecedence(Precedence precedence) {
    
}

static void expression() {
    parsePrecedence(PREC_ASSIGNMENT);
}

static uint8_t makeConstant(Value value) {
    // Adds the gien value to the end of the chunk's constant table and returns its index
    // then check that don't have too many constant because OP_CONSTANT instruction uses a single byte for the index operand
    // we can only store and load up to 260 constnat in a chunk
    int constant = addConstant(currentChunk(), value);
    if (constant > UINT8_MAX) {
        error("Too many constants in one chunk");
        return 0;
    }
    
    return (uint8_t)constant;
}

static void emitConstant(Value value) {
    // Add the vlaue to the constant table then emit op_constant and pushes it onto the stack at runtime    
    emitBytes(OP_CONSTANT, makeConstant(value));
}

static void number() { 
    // store a pointer to the following function
    // Assume that the token for the number literal has alredy been consumed nad i stored in 'previous'
    // Take the lexeme and use standar library to convert it to doubles
    double value = strtod(parser.previous.start, NULL);
    emitConstant(value);
}

static void grouping() {
    // The opening '(' has already been consumed.
    // We recursively compile the inner expression, then expect the closing ')'.
    // Grouping has no runtime semantics, it emits no bytecode of its own;
    // its only job is to lower precedence so the inner expression parses correctly.
    expression();
    consume(TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
}

static void unary() {
    // token has been consumed and is sitting in previous.
    // we grab the token type form thtat to note which unary operator we;'re dealing with
    TokenType operatorType = parser.previous.type;

    // compile the operand.
    parsePrecedence(PREC_UNARY);

    // Emit the operator instruction
    switch (operatorType) {
        case TOKEN_MINUS: 
            emitByte(OP_NEGATE); 
            break;
        default:
            // unreachable
            return; 
    }
}

bool compile(const char *source, Chunk *chunk) {
    initScanner(source);
    
    compilingChunk = chunk;
    parser.hadError = false;
    parser.panicMode = false;


    // 'primes the pump' on the scanner
    advance();
    expression();
    consume(TOKEN_EOF, "Expect end of expression.");
    endCompiler();
    grouping();
    return !parser.hadError;
}






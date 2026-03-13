#include <stdio.h>
#include <string.h>

#include "common.h"
#include "scanner.h"

typedef struct {
    // Marks the beginning of the current lexeme (token)
    const char* start;
    // Points to current character being looked at
    const char* current;
    int line;
} Scanner;

Scanner scanner;

void initScanner(const char* source) {
    // We start at the very first character on the very first line
    scanner.start = source;
    scanner.current = source;
    scanner.line = 1;
}
/**
 * Sacans the next token from the source.
 * 'start' marks the beginning of the lexeme and 'current' advances
 * as character are consumed. 
*/
Token scanToken() {
    skipWhitespace();

    // mark the beginning of the next lexeme
    scanner.start = scanner.current;

    if(isAtEnd()) 
        return makeToken(TOKEN_EOF);

    char c =  advance();

    switch(c) {
        case '(': return makeToken(TOKEN_LEFT_PAREN);
        case ')': return makeToken(TOKEN_RIGHT_PAREN);
        case '{': return makeToken(TOKEN_LEFT_BRACE);
        case '}': return makeToken(TOKEN_RIGHT_BRACE);
        case ';': return makeToken(TOKEN_SEMICOLON);
        case ',': return makeToken(TOKEN_COMMA);
        case '.': return makeToken(TOKEN_DOT);
        case '-': return makeToken(TOKEN_MINUS);
        case '+': return makeToken(TOKEN_PLUS);
        case '/': return makeToken(TOKEN_SLASH);
        case '*': return makeToken(TOKEN_STAR);
        case '!':
            return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=':
            return makeToken(match('=') ? TOKEN_BANG_EQUAL : TOKEN_EQUAL);
        case '<':
          return makeToken(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '>':
          return makeToken(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
        case '"': return string();
    }

    return errorToken("Unexpected character.");
}

static bool isAtEnd() {
    return *scanner.current == '\0';
}

/**
 * Creates a token using the characters between start and current
 * the lexeme is not copied; it points into the original source
 */
static Token makeToken(TokenType type) {
    Token token;
    token.type = type;
    token.start = scanner.start;
    // it uses scanner's start and current to capture token's lexeme
    token.length = (int)(scanner.current - scanner.start);
    token.line = scanner.line;
    return token;
}

static Token errorToken(const char* message) {
    Token token;
    token.type = TOKEN_ERROR;
    token.start = message;
    token.length = (int)strlen(message);
    token.line = scanner.line;
    return token;
}

/**
 * Consume the current character and move forward.
 */
static char advance() {
  scanner.current++;
  return scanner.current[-1];
}

static bool match(char expected) {
    if(isAtEnd()) return false;
    if(*scanner.current != expected) return false;

    // if current character is the desired one, we advance and return true;
    scanner.current++;
    return true;
}

static void skipWhitespace() {
    for (;;) {
        char c = peek();
        switch (c) {
            case ' ': 
            case '\r': 
            case '\t': 
            case '\n':
                scanner.line++;
                advance();
                break;
            case '/':
                // Comments start with // in Lox, if we don't find a second /, the skipWhitespace() needs to not consume the first slash neither.
                if(peekNext() == '/') {
                    // A comment goes until the end of the line
                    while (peek() != '\n' && !isAtEnd()) advance();
                } else {
                    return;
                }
                break;
            default:
                return;
        }
    }
}

/**
 * Returns the current character without consuming it.
 * (lookahead of 1 character)
 */
static char peek() {
    return *scanner.current;
}

/**
 * Returns the next character after the current one without consuming it.
 * (lookahead of 2 characters)
 */
static char peekNext() {
    if(isAtEnd()) return '\0';
    return scanner.current[1];
}
/**
 *  Consume characters until we reach the closing quote.
 *  track newlines inside the string literal
*/
static Token string() {
    while(peek() != '"' && !isAtEnd()) {
        if(peek() == '\n') scanner.line++;
        advance();
    }

    if(isAtEnd()) return errorToken("Undeterminated string.");

    // Closing quote
    advance();
    return makeToken(TOKEN_STRING);
}

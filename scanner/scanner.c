#include <string.h>
#include <stdbool.h>
#include "scanner.h"

typedef struct {
    // Marks the beginning of the current lexeme (token)
    // First token character
    const char* start;
    // Points to current character being looked at
    // First character that not belong to the token
    const char* current;
    int line;
} Scanner;

Scanner scanner;


// - Helpers - //
static bool isAtEnd() {
    return *scanner.current == '\0';
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

static bool isDigit(char c) {
    return c >= '0' && c <= '9';
}

/**
 * Check if a character is a minus or capitalize letter or undersoce 
 */
static bool isAlpha(char c) {
    // remember; char are saved as number base on ASCII
    // that's why we can compare using > or <
    return (c >= 'a' && c <= 'z') || 
           (c >= 'A' && c <= 'Z') || 
           (c == '_');
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
                // Comments start with '//' in Lox, if we don't find a second /, the skipWhitespace() needs to not consume the first slash neither.
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

// - Token Manipulation - //

/**
 * Creates a token using the characters between start and current
 * the lexeme is not copied; it points into the original source
 */
static Token makeToken(TokenType type) {
    Token token;
    token.type = type;
    token.start = scanner.start;
    // Determinate how many character is between current and start
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

static Token number() {
    while (isDigit(peek())) advance();

    // Look for a fractional part
    if (peek() == '.' && isDigit(peekNext())) {
        // Consume the '.'
        advance();

        while (isDigit(peek())) advance();
    }

    return makeToken(TOKEN_NUMBER);
}

static TokenType checkKeyword(int start, int length, const char* rest, TokenType type) {
    // the lexeme must be exactly as long as the keyword.
    // if first letter is 's', the lexeme could still be 'sup' or 'superb', remaining characters must match exactly
    if(scanner.current - scanner.start == start + length && memcmp(scanner.start + start, rest, length) == 0) {
        return type;
    }

    return TOKEN_IDENTIFIER;
}

static TokenType identifierType() {
    switch(scanner.start[0]) {
        case 'a': return checkKeyword(1, 2, "nd", TOKEN_AND);
        case 'c': return checkKeyword(1, 4, "lass", TOKEN_CLASS);
        case 'e': return checkKeyword(1, 3, "lse", TOKEN_ELSE);
        case 'f': 
            // If the difference between token is > 1 mean that there is still more character to check
            // so check the second character to distinguis between possibles keywords
            if(scanner.current - scanner.start > 1) {
                switch(scanner.start[1]) {
                    case 'a': return checkKeyword(2, 3, "lse", TOKEN_FALSE);
                    case 'o': return checkKeyword(2, 1, "r", TOKEN_FOR);
                    case 'u': return checkKeyword(2, 1, "n", TOKEN_FUN);
                }
            }
            break;
        case 'i':return checkKeyword(1, 1, "f", TOKEN_IF);
        case 'n': return checkKeyword(1, 2, "il", TOKEN_NIL);
        case 'o': return checkKeyword(1, 1, "r", TOKEN_OR);
        case 'p': return checkKeyword(1, 4, "rint", TOKEN_PRINT);
        case 'r': return checkKeyword(1, 5, "eturn", TOKEN_RETURN);
        case 's': return checkKeyword(1, 4, "uper", TOKEN_SUPER);
        case 't':
            // If the difference between token is > 1 mean that there is still more character to check
            // so check the second character to distinguis between possibles keywords
            if (scanner.current - scanner.start > 1) {
                switch (scanner.start[1]) {
                    case 'h': return checkKeyword(2, 2, "is", TOKEN_THIS);
                    case 'r': return checkKeyword(2, 2, "ue", TOKEN_TRUE);                    
                }
            }
            break;
        case 'v': return checkKeyword(1, 2, "ar", TOKEN_VAR);
        case 'w': return checkKeyword(1, 4, "hile", TOKEN_WHILE);
        }

    return TOKEN_IDENTIFIER;
}

static Token identifier() {
    // Mean while is letter or digit consume the character and move forward
    while (isAlpha(peek()) || isDigit(peek())) advance();

    return makeToken(identifierType());
}

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

    char c = advance();
    
    // scan lexeme start with letter or underscore
    if(isAlpha(c)) return identifier();
    // scan lexeme start with digit
    if(isDigit(c)) return number();

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
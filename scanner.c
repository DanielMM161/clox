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
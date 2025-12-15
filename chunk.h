#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"

typedef enum {
    OP_CONSTANT,
    OP_RETURN,
    OP_CONSTANT_LONG,
} OpCode;

typedef struct {    
    int count; // Repetitions
    int line; // Numbers of lines
} LineStart;

typedef struct {
    int count;
    int capacity;
    uint8_t* code;

    int lineCount;
    int lineCapacity;
    LineStart* lines;

    ValueArray constants;
} Chunk;

void initChunk(Chunk* chunk);
void freeChunk(Chunk* chunk);
void writeChunk(Chunk* chunk, uint8_t byte, int line);
void writeChunkLine(Chunk* chunk, int line);
int getLine(Chunk* chunk, int instruction);
int addConstant(Chunk* chunk, Value value);
void writeConstant(Chunk* chunk, Value value, int line);

#endif
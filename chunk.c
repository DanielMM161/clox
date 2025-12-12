#include <stdlib.h>

#include "chunk.h"
#include "memory.h"

void initChunk(Chunk* chunk) {
    // Code related
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    // Line related
    chunk->lineCount = 0;
    chunk->lineCapacity = 0;
    chunk->lines = NULL;
    initValueArray(&chunk->constants);
}

void writeChunk(Chunk* chunk, uint8_t byte, int line) {    
    if(chunk->capacity < chunk->count + 1) {
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);
        chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
    }

    chunk->code[chunk->count] = byte;    
    chunk->count++;

    writeChunkLine(chunk, line);
}

void writeChunkLine(Chunk* chunk, int line) {    
    if (chunk->lineCount > 0) {
        LineStart* last = &chunk->lines[chunk->lineCount - 1];
        if (last->line == line) {
            last->count++;
            return;
        }
    }
    
    if (chunk->lineCapacity < chunk->lineCount + 1) {
        int oldCapacity = chunk->lineCapacity;
        chunk->lineCapacity = GROW_CAPACITY(oldCapacity);        
        chunk->lines = GROW_ARRAY(LineStart, chunk->lines, oldCapacity, chunk->lineCapacity);
    }
    
    LineStart* newLine = &chunk->lines[chunk->lineCount];
    newLine->line = line;
    newLine->count = 1; 
    chunk->lineCount++;
}

int getLine(Chunk* chunk, int instruction) {
    int result = 0;

    for(int offset = 0; offset < chunk->lineCount; offset++) {        
        int linesCount = chunk->lines[offset].count;
        if(instruction >= linesCount) {
            instruction -= linesCount;
        } else {
            result = chunk->lines[offset].line;
            break;
        }
    }
    return result;
}

void freeChunk(Chunk* chunk) {
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    FREE_ARRAY(int, chunk->lines, chunk->capacity);
    freeValueArray(&chunk->constants);
    initChunk(chunk);
}

int addConstant(Chunk* chunk, Value value) {
    writeValueArray(&chunk->constants, value);
    return chunk->constants.count - 1;
}

void writeConstant(Chunk* chunk, Value value, int line) {
    int index = addConstant(chunk, value);

    // If the index fits in one byte, use the standard short op.
    if (index < 256) {
        writeChunk(chunk, OP_CONSTANT, line);
        writeChunk(chunk, (uint8_t)index, line);
    } else {
        // Index is too big, use the 24-bit instruction.
        writeChunk(chunk, OP_CONSTANT_LONG, line);

        // Split the 24-bit index into 3 bytes (little-endian).

        // 1. Lowest 8 bits
        writeChunk(chunk, (uint8_t)(index & 0xff), line);

        // 2. Middle 8 bits (shift right by 8)
        writeChunk(chunk, (uint8_t)((index >> 8) & 0xff), line);

        // 3. Highest 8 bits (shift right by 16)
        writeChunk(chunk, (uint8_t)((index >> 16) & 0xff), line);
    }
}
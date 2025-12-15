#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"

int main(int argc, const char* argv[]) {    
    // Init virtual machine
    initVM();
    Chunk chunk;
    initChunk(&chunk);    
    
    // Add a constant (1)
    int constant = addConstant(&chunk, 1);
    // Add an "constant" instruction
    writeChunk(&chunk, OP_CONSTANT, 123); 
    // Add the offset (where the constant is stored)
    writeChunk(&chunk, constant, 123);

    // Add a second constant (2)
    constant = addConstant(&chunk, 2);  
    // Add an "constant" instruction
    writeChunk(&chunk, OP_CONSTANT, 123);
    // Add the offset (where the constant is stored)
    writeChunk(&chunk, constant, 123);

    // Add an "add" instruction
    writeChunk(&chunk, OP_ADD, 123);
    
    // Add a third constant (6)
    constant = addConstant(&chunk, 6);
    // Add an "constant" instruction
    writeChunk(&chunk, OP_CONSTANT, 123);
    // Add the offset (where the constant is stored)
    writeChunk(&chunk, constant, 123);

    // Add an "divide" instruction
    writeChunk(&chunk, OP_DIVIDE, 123);
    
    // Add an "negate" instruction
    writeChunk(&chunk, OP_NEGATE, 123);
    
    // Add an "return" instruction
    writeChunk(&chunk, OP_RETURN, 123);
    
    disassembleChunk(&chunk, "test chunk");

    // Free virtual machione
    freeVM();
    interpret(&chunk);
    freeChunk(&chunk);
    return 0;
}
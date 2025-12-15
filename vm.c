#include "vm.h"
#include "chunk.h"
#include "common.h"
#include "debug.h"
#include <stdio.h>

VM vm;

void initVM() { resetStack(); }
void freeVM() {}
void resetStack() {
  // Aim to the begining of the array of vm.stack
  vm.stackTop = vm.stack;
}
void push(Value value) {
  // Add the value to the actual "empty box"
  *vm.stackTop = value;
  // Move the stack pointer to the next "empty box"
  vm.stackTop++;
}
Value pop() {
  // Move the stack pointer back to get to the most recent used slot in the
  // array
  vm.stackTop--;
  // Then look up the value at the inded and return it.
  // No need to explicityly "remove" it from the array
  return *vm.stackTop;
}

static InterpretResult run() {
    // Read the byte currently pointed at by ip and then advance the instruction
    // pointer
    #define READ_BYTE() (*vm.ip++)
    // Read the next byte of the bytecode, threat resulting number as a index
    // and looks up the corresponding Value in the chunk's constant table
    #define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])    
    #define BINARY_OP(op) \
        do { \
            double b = pop(); \
            double a = pop(); \
            push(a op b); \
        } while(false)    
    // Loop (Fetch-Decode-Execute)
    for (;;) {
        // This instruction dissasemble the instruction before to execute it
        // this disassemble dinamycally
        // vm.ip where I'am actually, vm.chunk->code where everything started
        // so to know where
        #ifdef DEBUG_TRACE_EXECUTION
            printf("        ");
            for (Value *slot = vm.stack; slot < vm.stackTop; slot++) {
                printf("[ ");
                printValue(*slot);
                printf(" ]");
            }
            printf("\n");
            disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
        #endif
        uint8_t instruction;
        switch (instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                Value constant = READ_CONSTANT();
                printValue(constant);
                printf("\n");
                break;
            }
            case OP_NEGATE: BINARY_OP(-); break;
            case OP_ADD: BINARY_OP(+); break;
            case OP_SUBTRACT: BINARY_OP(-); break;
            case OP_MULTIPLY: BINARY_OP(*); break;
            case OP_DIVIDE: BINARY_OP(/); break;
            case OP_RETURN: {
                printValue(pop());
                printf("\n");
                return INTERPRET_OK;
            }

        }
    }
    #undef READ_BYTE
    #undef READ_CONSTANT
    #undef BINARY_OP
}
InterpretResult interpret(Chunk *chunk) {
  vm.chunk = chunk;
  vm.ip = vm.chunk->code;
  return run();
}

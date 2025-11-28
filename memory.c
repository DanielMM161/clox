#include <stdlib.h>
#include "memory.h"

void* reallocate(void* pointer, size_t oldSize, size_t newSize) {
    if(newSize == 0) {
        free(pointer);
        return NULL;
    }

    void* results =  realloc(pointer, newSize);
    if(results == NULL) exit(1);

    return results;
}
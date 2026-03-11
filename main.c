#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vm.h"

static void repl() {
    char line[1024];
    for (;;) {
      	printf("> ");

    	if(!fgets(line, sizeof(line), stdin)) {
			printf("\n");
			break;
      	}
		interpret(line);
    }
}

static void runFile(const char* path) {
  char *source = myReadFile(path);
  InterpretResult result = interpret(source);
  // we nbeed to free the sopurce code string because readFile() dynamically
  // allocates it and passes ownership to its caller
  free(source);

  if (result == INTERPRET_COMPILE_ERROR)
    exit(65);
  if (result == INTERPRET_RUNTIME_ERROR)
    exit(65);
}

static char* myReadFile(const char* path) {
	// we open the file 	
	FILE* file = fopen(path, "rb");

	// if the file doesn't exist, we print an error message and exit
	if(file == NULL) {
		fprintf(stderr, "Could not open file \"%s\"\n", path);
		exit(74);
	}

	// Before reading it, we seek to the very end using fseek()
	fseek(file, 0L, SEEK_END);

	// ftell() tell us how many bytes we are from the start of the file
	size_t fileSize = ftell(file);
	// we rewind back to the beginning,a llocate a string of that size and read the whole file in a single batch
	rewind(file);

	char* buffer = (char*)malloc(fileSize + 1);
	// if we can't allocate enought memory to read the Lox script, we want to know it
	if(buffer == NULL) {
		fprintf(stderr, "Not enough memory to read \"%s\"\n", path);
		exit(74);
	}

	size_t bytesRead = fread(buffer, sizeof(char), fileSize, file);
	// if the number of bytes read is less than the file size, it means we couldn't read the whole file
	if(bytesRead < fileSize) {
		fprintf(stderr, "Could not read file \"%s\"\n", path);
		exit(74);
	}

	// add a null terminator to the end of the buffer
	buffer[bytesRead] = '\0';

	fclose(file);
	return buffer;

}

int main(int argc, const char* argv[]) {    
    // Init virtual machine
    initVM();

    if (argc == 1) {
      repl();
    } else if (argc == 2) {
      runFile(argv[1]);
    } else {
      fprintf(stderr, "Usage: clox [path]\n");
      exit(64);
    }

    return 0;
}
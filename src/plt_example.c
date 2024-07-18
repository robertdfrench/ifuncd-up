// This program can be used to demonstrate the PLT section of an ELF
// binary. To do so, you need to compile the program and then disassemble it.
//
// Compile: gcc -o plt_example plt_example.c
// Dissassemble: objdump -d plt_example
//
// This program contains calls to three dynamically resolved functions: strlen,
// write, and exit. Thus, the PLT in this program will contain three entries,
// one for each of these functions.
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
    const char *message = "Hello, World!\n";
    size_t len = strlen(message); // Here's one

    write(0, message, len); // Here's another
    exit(0); // And here's the last one
    return 0;
}

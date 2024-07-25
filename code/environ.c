// This program is used to show an example of how a GOT entry will look
// for an extern variable.
#include <stdio.h>

// This variable points to an array of strings, each containing an
// environment variable stored as a key/value pair. It is provided by
// glibc, and the linker will make it available to us if we declare that
// it like so:
extern char **environ;

int main() {
    printf("Address of environ: %p\n", (void*)&environ);
    printf("First environment variable: %s\n", environ[0]);
    return 0;
}

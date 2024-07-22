#include <stdio.h>

extern char **environ;

int main() {
    printf("Address of environ: %p\n", (void*)&environ);
    printf("First environment variable: %s\n", environ[0]);
    return 0;
}

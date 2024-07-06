#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main() {
    const char *message = "Hello, World!\n";
    size_t len = strlen(message);

    write(0, message, len);
    exit(0);
    return 0;
}

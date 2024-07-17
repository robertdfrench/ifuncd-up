#include <stdio.h>

__attribute__((visibility("default")))
int target_function(int x) {
    printf("Original target_function called\n");
    return x * 2;
}

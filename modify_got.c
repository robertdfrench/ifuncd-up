#define _GNU_SOURCE
#include <stdio.h>
#include <dlfcn.h>
#include <stdlib.h>

int replacement_function(int x) {
    printf("Replacement function called\n");
    return x * 3;
}

int main() {
    void *handle = dlopen("./modify_got_library.so", RTLD_LAZY);
    if (!handle) {
        fprintf(stderr, "Error opening library: %s\n", dlerror());
        return 1;
    }

    int (*target_function)(int) = dlsym(handle, "target_function");
    if (!target_function) {
        fprintf(stderr, "Error finding symbol: %s\n", dlerror());
        dlclose(handle);
        return 1;
    }

    printf("Address of target_function: %p\n", (void*)target_function);
    printf("Address of replacement_function: %p\n", (void*)replacement_function);

    printf("Original result: %d\n", target_function(5));

    // Get the address of the GOT entry
    void **got_entry = (void **)target_function;
    printf("Address of GOT entry: %p\n", (void*)got_entry);
    printf("Value in GOT entry: %p\n", *got_entry);

    // Modify the GOT entry to point to our replacement function
    printf("Attempting to modify GOT entry...\n");
    *got_entry = (void *)replacement_function;
    printf("GOT entry modified.\n");

    printf("Modified result: %d\n", target_function(5));

    dlclose(handle);
    return 0;
}

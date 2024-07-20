#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <termios.h>

// Function to print to a terminal
void print_to_tty(const char *message) {
    // ANSI escape code for green text
    const char *green_start = "\033[32m";
    const char *color_reset = "\033[0m";

    printf("%sTTY: %s%s\n", green_start, message, color_reset);
}

// Function to print to a file
void print_to_file(const char *message) {
    printf("FILE: %s\n", message);
}

// This is how you declare an IFUNC.
//
// This attribute tells gcc to invoke the "resolve_print_function" function
// during the link phase in order to determine which implementation of
// "print_message" should be used.
void print_message(const char *message) __attribute__((ifunc("resolve_print_function")));

// Resolver function
//
// This function returns a function pointer to the implementation we'd like to
// use for "print_message". 
void (*resolve_print_function(void))(const char *) {
    struct termios term;

    // Calling another dynamic function inside the resolver might not be safe in
    // general, but this example is very silly anyhow.
    int result = ioctl(STDOUT_FILENO, TCGETS, &term);
    if (result == 0) {
        // stdout is a terminal
        return print_to_tty;
    } else {
        // stdout is not a terminal
        return print_to_file;
    }
}

// Main function to test the print_message function
int main() {
    print_message("Hello, World!");
    return 0;
}

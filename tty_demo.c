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

// IFUNC attribute to use the resolver
void print_message(const char *message) __attribute__((ifunc("resolve_print_function")));

// Resolver function
void (*resolve_print_function(void))(const char *) {
    struct termios term;
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

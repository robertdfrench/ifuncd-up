// This program shows a simple example of using gnu ifunc. The idea is to create
// a "stub" function called "print_cpu_info". Whenever this stub function is
// called at runtime, it will point to one of the following real functions:
// 
// * print_avx2
// * print_sse42
// * print_cpu_sucks
//
// The decision about which of the real functions to call is made by the
// "resolve_cpu_info" function. That function is called by the dynamic linker as
// the program is being loaded loaded into memory, before "main" even starts.
//
// This is a simplified version of the code presented at
// https://jasoncc.github.io/gnu_gcc_glibc/gnu-ifunc.html.
#include <stdio.h>
#include <stddef.h>

// Run this code if the CPU supports AVX2
void print_avx2() {
	printf("AVX2 is present\n");
}

// Run this code if the CPU supports SSE4.2
void print_sse42() {
	printf("SSE4.2 is present\n");
}

// Run this code if the CPU supports neither of those
void print_cpu_sucks() {
	printf("This CPU is sucking my will to live!\n");
}

// This creates a stub function, which will call resolve_cpu_info() first to
// determine which of the above functions to run (based on CPU features). This
// choice is stored in something called the "Global Offset Table" so that the
// resolver function only has to be run once.
void print_cpu_info() __attribute__((ifunc ("resolve_cpu_info")));

// This is the "resolver" function. The dynamic linker will call this function
// when the program is being loaded in order to determine what function should
// actually be called when the program calls the "print_cpu_info" stub.
static void* resolve_cpu_info(void) {

        // This is to show that the resolver runs before main.
	printf("Running the Resolver\n");

	// Initialize the CPU feature checks
	__builtin_cpu_init();

        // Here we check for CPU features to determine which function should be
        // run when the program calls the stub function "print_cpu_info".
	if (__builtin_cpu_supports("avx2")) {
		return print_avx2;
	} else if (__builtin_cpu_supports("sse4.2")) {
		return print_sse42;
	} else {
		return print_cpu_sucks;
	}
}

int main() {
        // Show that the resolver runs before main.
        printf("Entering main\n");

        // Call whatever function that "resolve_cpu_info" has decided that we
        // should use here.
	print_cpu_info();
	return 0;
}

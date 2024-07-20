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

// This is the "resolver" function. The first time the program calls
// print_cpu_info(), it will call this function to determine which function
// really needs to be run.
static void* resolve_cpu_info(void) {
	printf("Running the Resolver\n");
	if (__builtin_cpu_supports("avx2")) {
		return print_avx2;
	} else if (__builtin_cpu_supports("sse4.2")) {
		return print_sse42;
	} else {
		return print_cpu_sucks;
	}
}

int main() {
        printf("Entering main\n");
	print_cpu_info();
	return 0;
}

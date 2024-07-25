#include <stdio.h>

// Declare the function pointer and the "real" implementations. We are
// pretending here that multiplication isn't available without SSE4.2.
static int (*triple)(int) = 0;
int triple_sse42(int n) { return 3 * n; }
int triple_plain(int n) { return n + n + n; }

// Make use of the global function pointer since we know it was resolved
// in main.
void print_fifteen() {
	int fifteen = triple(5);
	printf("%d\n", fifteen);
}

int main() {
	// GNU IFUNC resolves all functions up front anyways, so we will
	// incur the same startup cost by doing it in main.
	__builtin_cpu_init();
	if (__builtin_cpu_supports("sse4.2")) {
		triple = triple_sse42;
	} else {
		triple = triple_plain;
	}
	
	// Call a function that uses the resolved function pointer.
	print_fifteen();
	return 0;
}

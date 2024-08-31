#include <stddef.h>

// Use this incrementer algorithm if AVX2 is available.
int fancy_incrementer(int x) {
	return x + 1;
}

// Use this if AVX2 is not available. It's the same as above, because we don't
// actually rely on AVX2. 
int normal_incrementer(int x) {
	return x + 1;
}


// Select an "appropriate" incrementer based on CPU features. The actual choice
// doesn't matter in this case, we just need something for the resolver to do.
static void* resolver(void) {
	__builtin_cpu_init();
	if (__builtin_cpu_supports("avx2")) {
		return fancy_incrementer;
	} else {
		return normal_incrementer;
	}
}

// This is the ifunc "stub" function. The first time it is called, the
// `resolver` will be invoked in order to select an appropriate "real" function.
// Once the "real" function is selected, its address will be stored in the
// Global Offset Table. When this stub is invoked in the future, the PLT will
// cause the program to jump directly to the selected function.
int increment(int) __attribute__((ifunc ("resolver")));

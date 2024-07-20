// This program is part of an experiment to compare the performance of GNU IFUNC
// vs plain-old function pointers. Run `make rigorous_speed_demo` to see a full
// comparison of speeds.
//
// This particular program selects an "appropriate" incrementer function lazily,
// via the GNU IFUNC facility.  The choice of incrementer is irrelevant, since
// they are the same; our concern is the cost of invoking the chosen incrementer
// based on what strategy we use to select it.
#include <limits.h>
#include <stddef.h>
static int counter = 0;

// Use this incrementer algorithm if AVX2 is available.
void avx2_incrementer() {
	counter += 1;
}

// Use this if AVX2 is not available. It's the same as above, because we don't
// actually rely on AVX2. 
void normal_incrementer() {
	counter += 1;
}

// This is the ifunc "stub" function. The first time it is called, the
// `resolver` will be invoked in order to select an appropriate "real" function.
// Once the "real" function is selected, its address will be stored in the
// Global Offset Table. When this stub is invoked in the future, the PLT will
// cause the program to jump directly to the selected function.
void increment_counter() {
	if (__builtin_cpu_supports("avx2")) {
		avx2_incrementer();
	} else {
		normal_incrementer();
	}
}

int main() {
	// Count to ~ 2 Billion by calling a dynamically-resolved incrementer
	while (counter < INT_MAX) {
		increment_counter();
	}
	return 0;
}

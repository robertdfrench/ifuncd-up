// This program is part of an experiment to compare the performance of GNU IFUNC
// vs plain-old function pointers. Run `make rigorous_speed_demo` to see a full
// comparison of speeds.
//
// This particular program selects an "appropriate" incrementer function at the
// beginning of main, before any other work is done. The address of the chosen
// function is stored in a static function pointer.  The choice of incrementer
// is irrelevant, since they are the same; our concern is the cost of invoking
// the chosen incrementer based on what strategy we use to select it.
#include <limits.h>
#include <stddef.h>
#include <stdbool.h>
static int counter = 0;
static bool cpu_has_avx2 = false;

// Use this incrementer algorithm if AVX2 is available.
void avx2_incrementer() {
	counter += 1;
}

// Use this if AVX2 is not available. It's the same as above, because we don't
// actually rely on AVX2. 
void normal_incrementer() {
	counter += 1;
}

// Select an "appropriate" incrementer based on CPU features. The actual choice
// doesn't matter in this case, we just need something for the resolver to do.
void increment_counter() {
	if (cpu_has_avx2) {
		avx2_incrementer();
	} else {
		normal_incrementer();
	}
}

void detect_cpu_features() {
	cpu_has_avx2 = __builtin_cpu_supports("avx2");
}

int main() {
	detect_cpu_features();

	// Count to ~ 2 Billion by calling a dynamically-resolved incrementer
	while (counter < INT_MAX) {
		increment_counter();
	}
	return 0;
}

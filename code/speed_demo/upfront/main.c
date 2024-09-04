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
static bool cpu_has_avx2 = false;

int fancy_incrementer(int);
int normal_incrementer(int);

// Select an "appropriate" incrementer based on CPU features. The actual choice
// doesn't matter in this case, we just need something for the resolver to do.
int increment(int x) {
	if (cpu_has_avx2) {
		return fancy_incrementer(x);
	} else {
		return normal_incrementer(x);
	}
}

void detect_cpu_features() {
	__builtin_cpu_init();
	cpu_has_avx2 = __builtin_cpu_supports("avx2");
}

int main() {
	detect_cpu_features();

	// Count to ~ 2 Billion by calling a dynamically-resolved incrementer
        int counter = 0;
	while (counter < INT_MAX) {
		counter = increment(counter);
	}
	return 0;
}

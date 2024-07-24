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
static int counter = 0;

// This is a function pointer, which will eventually point to one of the two
// incrementer functions defined below. The choice of incrementer is determined
// by the resolver function. Once the "real" function is selected, its address
// is stored here.
static void (*increment_counter)(void) = 0;

// Use this incrementer algorithm if AVX2 is available.
void fancy_incrementer() {
	counter += 1;
}

// Use this if AVX2 is not available. It's the same as above, because we don't
// actually rely on AVX2. 
void normal_incrementer() {
	counter += 1;
}

// Select an "appropriate" incrementer based on CPU features. The actual choice
// doesn't matter in this case, we just need something for the resolver to do.
void (*resolver(void))(void) {
	__builtin_cpu_init();
	if (__builtin_cpu_supports("avx2")) {
		return fancy_incrementer;
	} else {
		return normal_incrementer;
	}
}

int main() {
	// Select the appropriate incrementer before doing any other work. This
	// kind of symbol resolution is "proactive" rather than "lazy".
	increment_counter = resolver();

	// Count to ~ 2 Billion by calling a dynamically-resolved incrementer
	while (counter < INT_MAX) {
		increment_counter();
	}
	return 0;
}

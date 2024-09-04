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

int fancy_incrementer(int);
int normal_incrementer(int);

int main() {
        __builtin_cpu_init ();
	// Count to ~ 2 Billion by calling a dynamically-resolved incrementer
        int counter = 0;
	while (counter < INT_MAX) {
		if (__builtin_cpu_supports("avx2")) {
			counter = fancy_incrementer(counter);
		} else {
			counter = normal_incrementer(counter);
		}
	}
	return 0;
}

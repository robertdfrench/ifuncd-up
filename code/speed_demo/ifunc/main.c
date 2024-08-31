// This program is part of an experiment to compare the performance of GNU IFUNC
// vs plain-old function pointers. Run `make rigorous_speed_demo` to see a full
// comparison of speeds.
//
// This particular program selects an "appropriate" incrementer function lazily,
// via the GNU IFUNC facility.  The choice of incrementer is irrelevant, since
// they are the same; our concern is the cost of invoking the chosen incrementer
// based on what strategy we use to select it.
#include <limits.h>

int increment(int);

int main() {
	// Count to ~ 2 Billion by calling a dynamically-resolved incrementer
	int counter = 0;
	while (counter < INT_MAX) {
		counter = increment(counter);
	}
	return 0;
}

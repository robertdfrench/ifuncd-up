// This program is part of an experiment to compare the performance of GNU IFUNC
// vs plain-old function pointers. Run `make rigorous_speed_demo` to see a full
// comparison of speeds.
//
// This particular program acts as a control, allowing us to measure how long it
// takes to count from 0 to INT_MAX. The performance of the other two programs
// should be compared to the performance of this one.
#include <limits.h>

int main() {
	int counter = 0;
	// Count to ~ 2 Billion in a tight loop.
	while (counter < INT_MAX) {
		counter += 1;
	}
	return 0;
}

#include <limits.h>
#include <stddef.h>
static int counter = 0;
static void (*increment_counter)(void) = 0;

void fancy_incrementer() {
	counter += 1;
}

void normal_incrementer() {
	counter += 1;
}

void (*resolver(void))(void) {
	if (__builtin_cpu_supports("avx2")) {
		return fancy_incrementer;
	} else {
		return normal_incrementer;
	}
}

int main() {
	increment_counter = resolver();

	while (counter < INT_MAX) {
		increment_counter();
	}
	return 0;
}

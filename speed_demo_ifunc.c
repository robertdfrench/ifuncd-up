#include <limits.h>
#include <stddef.h>
static int counter = 0;

void fancy_incrementer() {
	counter += 1;
}

void normal_incrementer() {
	counter += 1;
}

static void* resolver(void) {
	if (__builtin_cpu_supports("avx2")) {
		return fancy_incrementer;
	} else {
		return normal_incrementer;
	}
}

void increment_counter() __attribute__((ifunc ("resolver")));

int main() {
	while (counter < INT_MAX) {
		increment_counter();
	}
	return 0;
}

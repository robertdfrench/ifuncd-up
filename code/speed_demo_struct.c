#include <err.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
static int counter = 0;

struct myfuncs {
	void (*increment_counter)();
};

static struct myfuncs *page;


// Use this incrementer algorithm if AVX2 is available.
void fancy_incrementer() {
	counter += 1;
}

// Use this if AVX2 is not available. It's the same as above, because we don't
// actually rely on AVX2. 
void normal_incrementer() {
	counter += 1;
}

void initfuncs(void) {
	int pagesize = getpagesize();
	page = mmap(NULL, pagesize, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, 0, 0);

	if (page == MAP_FAILED)
		err(123, "mmap");
	

	__builtin_cpu_init();
	if (__builtin_cpu_supports("avx2")) {
		page->increment_counter = fancy_incrementer;
	} else {
		page->increment_counter = normal_incrementer;
	}

	if(mprotect(page, pagesize, PROT_READ))
		err(123, "mprotect");

}


int main(void) {
	initfuncs();

	while (counter < INT_MAX) {
		page->increment_counter();
	}

	return 0;
}

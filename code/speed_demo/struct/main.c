#include <err.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>
struct myfuncs {
	int (*increment)(int);
};

static struct myfuncs *page;


int fancy_incrementer(int);
int normal_incrementer(int);

void initfuncs(void) {
	int pagesize = getpagesize();
	page = mmap(NULL, pagesize, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANON, 0, 0);

	if (page == MAP_FAILED)
		err(123, "mmap");
	

	__builtin_cpu_init();
	if (__builtin_cpu_supports("avx2")) {
		page->increment = fancy_incrementer;
	} else {
		page->increment = normal_incrementer;
	}

	if(mprotect(page, pagesize, PROT_READ))
		err(123, "mprotect");

}


int main(void) {
	initfuncs();

        int counter = 0;
	while (counter < INT_MAX) {
		counter = page->increment(counter);
	}

	return 0;
}

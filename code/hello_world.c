// This program is used to demonstrate the fact that binaries have
// metadata stored in them that tells the linker which dynamic libraries
// they need in order to run.
#include <stdio.h>

int main(int argc, char** argv) {
	// We have to make printf do something non-trivial here, or the
	// compiler will simplify it to "puts" instead. That would just
	// make th example more confusing. So we have it print something
	// that will always be available: the name of the running
	// program.
	printf("Hello from %s!\n", argv[0]);
	return 0;
}

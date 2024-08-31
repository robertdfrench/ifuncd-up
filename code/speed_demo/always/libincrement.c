// Use this incrementer algorithm if AVX2 is available.
int fancy_incrementer(int x) {
	return x + 1;
}

// Use this if AVX2 is not available. It's the same as above, because we don't
// actually rely on AVX2. 
int normal_incrementer(int x) {
	return x + 1;
}

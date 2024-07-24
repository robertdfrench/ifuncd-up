#include <smmintrin.h>
#include <stdio.h>

void vector_add_sse42(float *a, float *b, float *c, int n) {
	int i;
	printf("Implementation: sse4.2\n");
	for (int i = 0; i <= n - 4; i += 4) {
		__m128 va = _mm_loadu_ps(&a[i]);
		__m128 vb = _mm_loadu_ps(&b[i]);
		__m128 vc = _mm_add_ps(va, vb);
		_mm_storeu_ps(&c[i], vc);
	}
	// Handle the remaining elements
	for (; i < n; i++) {
		c[i] = a[i] + b[i];
	}
}

void vector_add_plain(float *a, float *b, float *c, int n) {
	printf("Implementation: plain\n");
	for (int i = 0; i < n; i++) {
		c[i] = a[i] + b[i];
	}
}

void vector_add(float*, float*, float*, int)
	__attribute__((ifunc("resolve_vector_add")));

void (*resolve_vector_add(void))(float*, float*, float*, int) {
	__builtin_cpu_init();
	if (__builtin_cpu_supports("sse4.2")) {
		return vector_add_sse42;
	} else {
		return vector_add_plain;
	}
}

int main() {
	float a[16] = {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0, 9.0, 10.0, 11.0, 12.0, 13.0, 14.0, 15.0, 16.0};
	float b[16] = {16.0, 15.0, 14.0, 13.0, 12.0, 11.0, 10.0, 9.0, 8.0, 7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0};
	float c[16];
	vector_add(a, b, c, 16);

	for (int i = 0; i < 16; i++) {
		printf("%f ", c[i]);
	}
	printf("\n");

	return 0;
}

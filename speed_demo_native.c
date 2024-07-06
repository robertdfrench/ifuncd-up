static int counter = 0;
const int limit = (2047) * 1024 * 1024;

void function_a() {
	counter += 1;
}

void function_b() {
	counter += 1;
}

void (*resolver(void))(void) {
	if (counter == 0) {
		return function_a;
	} else {
		return function_b;
	}
}

int main() {
	void (*increment_counter)(void) = resolver();

	while (counter < limit) {
		increment_counter();
	}
	return 0;
}

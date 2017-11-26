#include <stdio.h>
#include "polyline.h"

int main(int argc, char *argv[]) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <polyline>\n", argv[0]);
		return 1;
	}
	float *result;

	int r = polyline_decode(&result, argv[1]);
	if (r < 0) {
		printf("ERROR!\n");
		return 0;
	}
	printf("[\n");
	for (int i = 0; i < r; i++) {
		printf("    [%.4f, %.4f],\n", result[i * 2], result[i * 2 + 1]);
	}
	printf("]\n");
	return 0;
}

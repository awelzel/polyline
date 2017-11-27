#include <stdio.h>
#include "polyline.h"

/*
 * TODO: support encode and decode?
 *
 *   polyline encode str (where str can be any sort of array of floats in
 *                        textual form) preprocess by removing everyting
 *   polyline decode <s>
 *     - decode a string
 */

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
	printf("[");
	for (int i = 0; i < r; i++) {
		printf("[%.5f, %.5f]%s",
		       result[i * 2], result[i * 2 + 1],
		       (i < (r - 1)) ? ", " : ""
		);
	}
	printf("]\n");
	if (result) {
		free(result);
	}
	return 0;
}

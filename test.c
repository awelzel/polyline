#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "polyline.h"

static int
decode_print_compare(const char *polyline, const float *expected, size_t n)
{
	float *result;
	int r;
	printf("POLYLINE        %s\n", polyline);
	if ((r = polyline_decode(&result, polyline)) >= 0) {

		if (r != n) {
			printf("mismatch of decoded coordinates!\n");
			abort();
		}
		printf("EXPECTED RESULT ");
		printf("[");
		for (int i = 0; i < n; i++) {
			printf("[%.5f, %.5f]%s%s",
				expected[i * 2], expected[i * 2 + 1],
				(i == n - 1) ? "" : ",",
				(i == n - 1) ? "]\n" : " ");
		}
		printf("DECODE RESULT   ");
		printf("[");
		for (int i = 0; i < r; i++) {
			printf("[%.5f, %.5f]%s%s",
				result[i * 2], result[i * 2 + 1],
				(i == n - 1) ? "" : ",",
				(i == r - 1) ? "]\n" : " ");
		}

		free(result);
	} else {
		printf("polyline_decode failed!\n");
		return -1;
	}
	return 0;
}


int
main(int argc, char *argv[])
{
	char *result;
	int r;
	printf("Test 0...\n");
	const float data0[][2] = {
		{-5.00000f, 5.0000f},
		{-5.00000f, 5.0000f},
		{5.00000f, -5.0000f},
		{5.00000f, -5.0000f},
	};
	r = polyline_encode(&result, data0, 2);
	if (r != strlen(result)) {
		printf("strlen and return value disagree!\n");
		abort();
	}
	decode_print_compare(result, &data0[0][0], 2);
	free(result);

	printf("Test 1...\n");
	const float data1[][2] = {
		{38.50000f, -120.2000f},
		{40.70000f, -120.950000f},
		{43.2520000f, -126.4530000f},
	};
	r = polyline_encode(&result, data1, 3);
	if (r != strlen(result)) {
		printf("strlen and return value disagree!\n");
		abort();
	}
	decode_print_compare(result, &data1[0][0], 3);
	free(result);

	printf("Test 2...\n");
	const float data2[][2] = {
		{180.0000f, -120.2000f},
		{-180.00000f, 120.950000f},
		{0.000f, -180.0},
	};
	r = polyline_encode(&result, data2, 3);
	if (r != strlen(result)) {
		printf("strlen and return value disagree!\n");
		abort();
	}
	decode_print_compare(result, &data2[0][0], 3);
	free(result);

	printf("Test 3...\n");
	const float data3[][2] = {
		{180.0f, 120.7f},
		{180.02f, 120.95f},
		{188.07f, 121.0},
	};
	r = polyline_encode(&result, data3, 3);
	if (r != strlen(result)) {
		printf("strlen and return value disagree!\n");
		abort();
	}
	decode_print_compare(result, &data3[0][0], 3);
	free(result);


	printf("Test 4...\n");
	const float data4[][2] = {
		{-82.31549f, -45.13815f},
		{4.07058f, 51.32793f},
		{-51.66234f, 33.53401f},
		{54.44179f, 42.62945f},
		{33.68633f, -36.11884f},
		{45.32522f, -7.28373f},
		{85.81230f, -32.38735f},
		{-30.40201f, -8.85930f},
	};
	r = polyline_encode(&result, data4, 8);
	if (r != strlen(result)) {
		printf("strlen and return value disagree!\n");
		abort();
	}
	decode_print_compare(result, &data4[0][0], 8);
	free(result);

	printf("Test 5...\n");
	const float data5[][2] = {
		{0.0, 0.0},
	};
	r = polyline_encode(&result, data5, 1);
	if (r != strlen(result)) {
		printf("strlen and return value disagree!\n");
		abort();
	}
	decode_print_compare(result, &data5[0][0], 1);
	free(result);

	printf("Test 6...\n");
	const float data6[0][2];
	r = polyline_encode(&result, data6, 0);
	if (r != strlen(result)) {
		printf("strlen and return value disagree!\n");
		abort();
	}
	decode_print_compare(result, &data6[0][0], 0);
	free(result);
}

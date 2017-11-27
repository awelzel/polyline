#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "polyline.h"

#ifdef DEBUG
#define dprintf(...) fprintf(stderr, __VA_ARGS__)
#define dprint_coords(...) print_coords(stderr, __VA_ARGS__)
#else
#define dprintf(...) do { } while(0);
#define dprint_coords(...) do { } while(0);
#endif

static float max_delta = 0.0001f;

static void
print_coords(FILE *f, const char *prefix, const float *coords, size_t n)
{
	printf("%s [", prefix);
	for (size_t i = 0; i < n; i++) {
		fprintf(f, "[%.7f, %.7f]%s%s",
			coords[i * 2], coords[i * 2 + 1],
			(i == n - 1) ? "" : ",",
			(i == n - 1) ? "]\n" : " ");
	}
}


static int
decode_print_compare(const char *polyline, const float *expected, size_t n)
{
	float *result = NULL;
	size_t size = 0;
	int r, delta_issue = 0;
	dprintf("POLYLINE        \"%s\"\n", polyline);
	r = polyline_decode(&result, &size, polyline);
	if (r < 0) {
		printf("ERROR decoding: %d\n", r);
		return -1;
	} else if ((size_t)r != n) {
		printf("ERROR decoding: r=%d != n=%lu\n", r, n);
		return -1;
	}

	for (size_t i = 0; i < n; i++) {
		float delta = fabsf(result[i] - expected[i]);
		if (delta > max_delta) {
			printf("ERROR: delta=%.7f expected[%lu]=%.7f result[%lu]=%.7f\n",
					delta, i, expected[i], i, result[i]);
			delta_issue = 1;
		}
	}
	if (delta_issue) {
		print_coords(stdout, "EXPECTED", expected, n);
		print_coords(stdout, "RESULT  ", result, n);
	}
	dprint_coords("DEBUG EXPECTED", expected, n);
	dprint_coords("DEBUG RESULT  ", result, n);

	free(result);
	return delta_issue ? -1 : 0;
}

static void
test_run(char *name, const float *coords, size_t n)
{
	char *result = NULL;
	size_t size = 0;
	int r;
	printf("Running %-30s", name);
	r = polyline_encode(&result, &size, coords, n);
	if (r < 0) {
		printf("ERROR: polyline_encode(): %d\n", r);
		return;
	} else if ((size_t)r != strlen(result)) {
		printf("ERROR: polyline_encode() strlen and disagree!\n");
		return;
	}
	if (!decode_print_compare(result, coords, n)) {
		printf("GOOD\n");
	}
	free(result);
}

static void
test_buffer_reuse(const float *data0, size_t n0, const float *data1, size_t n1)
{
	int r;
	size_t size = 0;
	char *result = NULL;
	printf("Running %-30s", "buffer reuse test");
	r = polyline_encode(&result, &size, data0, n0);
	dprintf("\n%s: r=%d size=%lu result=%p result='%s'\n",
			__FUNCTION__, r, size, result, result);
	r = polyline_encode(&result, &size, data1, n1);
	dprintf("%s: r=%d size=%lu result=%p result='%s'\n",
			__FUNCTION__, r, size, result, result);
	r = polyline_encode(&result, &size, data0, n0);
	dprintf("%s: r=%d size=%lu result=%p result='%s'\n",
			__FUNCTION__, r, size, result, result);

	size_t dsize = 0;
	float *dresult = NULL;
	r = polyline_decode(&dresult, &dsize, result);
	dprintf("%s: r=%d dsize=%lu dresult=%p\n", __FUNCTION__, r, dsize, dresult);
	r = polyline_decode(&dresult, &dsize, result);
	dprintf("%s: r=%d dsize=%lu dresult=%p\n", __FUNCTION__, r, dsize, dresult);

	free(result);
	free(dresult);
	printf("\n");
}

static void
test_polyline_decode_error(char *name, int expected_error,
		float *dst, size_t size, const char *polyline)
{
	/* TODO */
/*
	if (expected_encode_error) {
		if (r == expected_encode_error) {
			printf("GOOD: Expected encode error %d happened.\n",
				expected_encode_error);
			return;
		}
	}
	*/
}

static void
test_polyline_encode_error(char *name, int expected_error,
		float *dst, size_t size, const float *coords)
{
	/* TODO */
}

int
main(int argc, char *argv[])
{

	test_polyline_decode_error(
		"Test error with dst set but size == 0",
		POLYLINE_EINVAL,
		(float *)0xdeadbeef,
		0,
		"??"
	);

	const float data0[][2] = {
		{-5.00000f, 5.0000f},
		{-5.00000f, 5.0000f},
		{5.00000f, -5.0000f},
		{5.00000f, -5.0000f},
	};
	test_run("Test 0", &data0[0][0], 4);

	const float data1[][2] = {
		{38.50000f, -120.2000f},
		{40.70000f, -120.950000f},
		{43.2520000f, -126.4530000f},
	};
	test_run("Test 1", &data1[0][0], 3);

	const float data2[][2] = {
		{180.0000f, -120.2000f},
		{-180.00000f, 120.950000f},
		{0.000f, -180.0},
	};
	test_run("Test 2", &data2[0][0], 3);

	const float data3[][2] = {
		{180.0f, 120.7f},
		{180.02f, 120.95f},
		{188.07f, 121.0},
	};
	test_run("Test 3", &data3[0][0], 3);

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

	test_run("Test 4", &data4[0][0], 8);

	const float data5[][2] = {
		{0.0, 0.0},
	};
	test_run("Test 5", &data5[0][0], 1);

	const float data6[][2] = {
		{0.0, 0.0},
		{1.0, 1.0},
		{2.0, 2.0},
	};
	test_run("Test 6", &data6[0][0], 3);


	test_buffer_reuse(&data4[0][0], 8, &data1[0][0], 3);
}

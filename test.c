#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "polyline.h"

#ifdef DEBUG
#define dprintf(...) fprintf(stdout, __VA_ARGS__)
#define dprint_coords(...) print_coords(stdout, __VA_ARGS__)
#else
#define dprintf(...) do { } while(0);
#define dprint_coords(...) do { } while(0);
#endif

static float max_delta = 1 / 100000.0f;
static int test_name_indent = 50;

int
assert_int_equal(char const *msg, int expected, int result) {
	if (expected - result)
		printf("ERROR: %s expected=%d != result=%d\n",
			msg, expected, result);
	return expected - result;
}

int
assert_ptr_equal(char const *msg, const void *expected, const void *result)
{
	int diff = (char *)expected - (char *)result;
	if (diff)
		printf("ERROR: %s %p != %p\n", msg, expected, result);
	return diff;
}

int
assert_size_t_equal(char const *msg, size_t expected, size_t result)
{
	int diff = expected - result;
	if (diff)
		printf("ERROR: %s %lu != %lu\n", msg, expected, result);
	return diff;
}

int
assert_size_t_gt(char const *msg, size_t than, size_t result)
{
	if (result <= than) {
		printf("ERROR: %s %lu not greater than %lu\n", msg, result, than);
		return -1;
	}
	return 0;
}

int
assert_int_gt(char const *msg, int than, int result)
{
	if (result <= than) {
		printf("ERROR: %s %d not greater than %d\n", msg, result, than);
		return -1;
	}
	return 0;
}

int
assert_float_equal(char const *msg, float e, float result, float expected)
{
	float delta = fabsf(expected - result);
	if (delta >= e) {
		printf("ERROR: %s %.5f != %.5f\n", msg, expected, result);
		return -1;
	}
	return 0;
}

int
assert_str_equal(char const *msg, const char* expected, const char* result)
{
	if (strcmp(expected, result)) {
		printf("ERROR: %s '%s' != '%s'\n", msg, expected, result);
		return -1;
	}
	return 0;
}

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
		print_coords(stdout, "ERROR EXPECTED", expected, n);
		print_coords(stdout, "ERROR RESULT  ", result, n);
	}
	dprint_coords("DEBUG EXPECTED", expected, n);
	dprint_coords("DEBUG RESULT  ", result, n);

	free(result);
	return delta_issue ? -1 : 0;
}

static void
test_run(char *name, const float *coords, size_t n, char *expected_polyline)
{
	char *result = NULL;
	size_t size = 0;
	int r;
	printf("Running %-*s", test_name_indent, name);
	r = polyline_encode(&result, &size, coords, n);
	if (r < 0) {
		printf("ERROR: polyline_encode(): %d\n", r);
		return;
	} else if ((size_t)r != strlen(result)) {
		printf("ERROR: polyline_encode() strlen and disagree!\n");
		return;
	}

	if (expected_polyline && assert_str_equal(name, expected_polyline, result))
		return;

	if (!decode_print_compare(result, coords, n)) {
		printf("GOOD\n");
	}
	free(result);
}

static void
test_encode_buffer_reuse(void)
{
	const float data0[][2] = {
		{38.50000f, -120.2000f},
		{40.70000f, -120.950000f},
		{43.2520000f, -126.4530000f},
	};
	size_t n0 = 3;

	const float data1[][2] = {
		{-82.31549f, -45.13815f},
		{4.07058f, 51.32793f},
		{-51.66234f, 33.53401f},
		{54.44179f, 42.62945f},
		{33.68633f, -36.11884f},
		{45.32522f, -7.28373f},
		{85.81230f, -32.38735f},
		{-30.40201f, -8.85930f},
	};
	size_t n1 = 8;
	int r1, r2, r3;
	size_t size = 0, old_size = 0;
	char *result = NULL, *old_result = NULL;
	printf("Running %-*s", test_name_indent, __FUNCTION__);


	r1 = polyline_encode(&result, &size, &data0[0][0], n0);
	assert_int_equal("length", 27, r1);
	old_result = result;
	old_size = size;
	r2 = polyline_encode(&result, &size, &data1[0][0], n1);
	if (assert_int_gt("encode error?", 0, r2))
		return;

	if (assert_size_t_gt("buffer size not increased?", old_size, size))
		return;

	old_result = result;
	old_size = size;
	r3 = polyline_encode(&result, &size, &data0[0][0], n0);
	if (assert_int_gt("encode error?", 0, r3))
		return;
	if (assert_ptr_equal("encode buffer not reused?", old_result, result))
		return;
	if (assert_size_t_equal("encode buffer not reused?", old_size, size))
		return;

	free(result);
	printf("GOOD\n");
}

static void
test_decode_buffer_reuse(void)
{
	int r1, r2, r3;
	char *polyline1 = "??";
	char *polyline2 = "_p~iF~ps|U_ulLnnqC_mqNxxq`@";
	size_t size = 0, old_size = 0;
	float *result = NULL, *old_result = NULL;
	printf("Running %-*s", test_name_indent, __FUNCTION__);

	r1 = polyline_decode(&result, &size, polyline1);
	assert_int_equal("wrong coordinate count", 1, r1);
	old_result = result;
	old_size = size;

	r2 = polyline_decode(&result, &size, polyline1);
	assert_int_equal("wrong coordinate count", 1, r2);

	if (assert_ptr_equal("decode buffer not reused?", old_result, result))
		return;

	if (assert_size_t_equal("decode buffer not reused?", old_size, size))
		return;

	r3 = polyline_decode(&result, &size, polyline2);
	if (assert_int_equal("wrong coordinate count", 3, r3))
		return;
	if (assert_size_t_gt("decode buffer not reused?", old_size, size))
		return;

	free(result);
	printf("GOOD\n");
}

static void
test_strerror(void)
{
	printf("Running %-*s", test_name_indent, "strerror test"); fflush(stdout);
	if (assert_str_equal("wrong error message", polyline_strerror(POLYLINE_ENOMEM), "POLYLINE_ENOMEM"))
		return;
	if (assert_ptr_equal("no error message", polyline_strerror(0), NULL))
		return;
	printf("GOOD\n");
}

static void
test_polyline_decode(char *name, int expected_error,
		float *rptr, size_t rsize, const char *polyline)
{
	int r, free_rptr = 0;
	if (!rptr) /* we only free pointers allocated by polyline_decode() */
		free_rptr = 1;
	/* This probably leaks memory... */
	printf("Running %-*s", test_name_indent, name); fflush(stdout);
	r = polyline_decode(&rptr, &rsize, polyline);
	if (expected_error) {
		if (assert_int_equal("bad error code", expected_error, r))
			goto free;
	}
	printf("GOOD\n");

free:
	if (free_rptr && rptr)
		free(rptr);
}

static void
test_polyline_encode(char *name, int expected_error,
		char *rptr, size_t rsize, const float *coords, size_t n)
{
	int r, free_rptr = 0;
	if (!rptr) /* we only free pointers allocated by polyline_decode() */
		free_rptr = 1;

	printf("Running %-*s", test_name_indent, name); fflush(stdout);
	r = polyline_encode(&rptr, &rsize, coords, n);
	if (expected_error) {
		if (assert_int_equal("expected error code", expected_error, r))
			goto free;
	} else {
		/* the string is always longer than the number of coordinates */
		if (assert_int_gt("short string?", n, r))
			goto free;
	}

	printf("GOOD\n");

free:
	if (free_rptr && rptr)
		free(rptr);
}

int
main()
{
	const float google_polyline_data[][2] = {
		{38.50000f, -120.2000f},
		{40.70000f, -120.950000f},
		{43.2520000f, -126.4530000f},
	};
	const float data0[][2] = {
		{-5.00000f, 5.0000f},
		{-5.00000f, 5.0000f},
		{5.00000f, -5.0000f},
		{5.00000f, -5.0000f},
	};

	char *polyline0 = "~po]_qo]??_c`|@~b`|@??";
	test_run("Test 0", &data0[0][0], 4, polyline0);

	const float *data1 = &google_polyline_data[0][0];
	char *polyline1 = "_p~iF~ps|U_ulLnnqC_mqNxxq`@";
	test_run("Test 1", data1, 3, polyline1);

	const float data2[][2] = {
		{180.0000f, -120.2000f},
		{-180.00000f, 120.950000f},
		{0.000f, -180.0},
	};
	char *polyline2 = "_gsia@~ps|U~ngtcAorz~l@_gsia@rhzkx@";
	test_run("Test 2", &data2[0][0], 3, polyline2);

	const float data3[][2] = {
		{180.0f, 120.7f},
		{180.02f, 120.95f},
		{188.07f, 121.0},
	};
	char *polyline3 = "_gsia@_fu_V_|Boyo@ogcp@owH";
	test_run("Test 3", &data3[0][0], 3, polyline3);

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
	char *polyline4 = "xfluNl`orG_hgnO_`xkQvidsI~jrkB{mrfSomov@rxt}Bxos_Naf`fAmz~nDgsbvFr`fxCnbidUiirnC";
	float old_max_delta = max_delta;

	/* Reset delta because this is crazy random data...*/
	max_delta = 0.0001f;
	test_run("Test 4", &data4[0][0], 8, polyline4);
	max_delta = old_max_delta;

	const float data5[][2] = {
		{0.0, 0.0},
	};
	char *polyline5 = "??";
	test_run("Test 5", &data5[0][0], 1, polyline5);

	const float data6[][2] = {
		{0.0, 0.0},
		{1.0, 1.0},
		{2.0, 2.0},
	};
	char *polyline6 = "??_ibE_ibE_ibE_ibE";
	test_run("Test 6", &data6[0][0], 3, polyline6);

	test_polyline_decode("proper decoding.",
			0, (float *)0, 0, "??");
	test_polyline_decode("proper decoding.",
			0, (float *)0, 0, "{{{?}}}?");
	test_polyline_decode("decode error - rptr set, size 0",
			POLYLINE_EINVAL, (float *)0xdeadbeef, 0, "??");
	test_polyline_decode("decode error - size set, rptr NULL",
			POLYLINE_EINVAL, (float *)NULL, 32, "??");
	test_polyline_decode("decode error - polyline NULL",
			POLYLINE_EINVAL, (float *)0, 0, NULL);

	test_polyline_decode("decode error - polyline bad chars low",
			POLYLINE_EPARSE, (float *)0, 0, "??00");
	test_polyline_decode("decode error - polyline bad chars - binary crap",
			POLYLINE_EPARSE, (float *)0, 0, "\x7f\x80");
	test_polyline_decode("decode error - polyline truncated",
			POLYLINE_ETRUNC, (float *)0, 0, "?~");
	test_polyline_decode("decode error - polyline truncated - missing coord",
			POLYLINE_ETRUNC, (float *)0, 0, "a?");
	test_polyline_decode("decode error - polyline truncated - missing coord",
			POLYLINE_ETRUNC, (float *)0, 0, "_?");
	test_polyline_decode("decode error - polyline incomplete chunks 1",
			POLYLINE_ETRUNC, (float *)0, 0, "_");
	test_polyline_decode("decode error - polyline incomplete chunks 2",
			POLYLINE_ETRUNC, (float *)0, 0, "{{{?}}}");
	test_polyline_decode("decode error - polyline incomplete chunks 3",
			POLYLINE_ETRUNC, (float *)0, 0, "{{{?}}}?}}}");
	test_polyline_decode("decode error - polyline incomplete chunks 4",
			POLYLINE_ETRUNC, (float *)0, 0, "a?a?_");
	test_polyline_decode("decode error - polyline good chunks 1",
			0, (float *)0, 0, "a?a?_?_?");


	test_polyline_encode("proper encoding",
		0, (char *)0, 0, &google_polyline_data[0][0], 3);
	test_polyline_encode("encode error - rptr set, size 0",
		POLYLINE_EINVAL, (char *)0xdeadbeef, 0,
		&google_polyline_data[0][0], 3);
	test_polyline_encode("encode error - size set, rptr NULL",
		POLYLINE_EINVAL, (char *)0, 4,
		&google_polyline_data[0][0], 3);
	test_polyline_encode("encode error - coords NULL",
		POLYLINE_EINVAL, (char *)0, 0,
		NULL, 3);
	test_polyline_encode("encode error - n 0",
		POLYLINE_EINVAL, (char *)0, 0,
		&google_polyline_data[0][0], 0);


	test_encode_buffer_reuse();
	test_decode_buffer_reuse();

	test_strerror();

	return 0;
}

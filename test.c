#include <stdio.h>
#include "polyline.h"


int
main(int argc, char *argv[])
{
	printf("Test 0...\n");
	const float data0[][2] = {
		{5.00000f, -5.0000f},
		{5.00000f, -5.0000f},
	};
	char *result0 = polyline_encode(data0, 2);
	printf("RESULT0 = %s\n", result0);
	float *dresult0 = polyline_decode(result0);

	printf("Test 1...\n");
	const float data1[][2] = {
		{38.50000f, -120.2000f},
		{40.70000f, -120.950000f},
		{43.2520000f, -126.4530000f},
	};
	char *result1 = polyline_encode(data1, 3);
	printf("RESULT1 = %s\n", result1);

	float *dresult1 = polyline_decode(result1);

	printf("Test 2...\n");
	const float data2[][2] = {
		{180.0000f, -120.2000f},
		{-180.00000f, 120.950000f},
		{0.000f, -180.0},
	};
	char *result2 = polyline_encode(data2, 3);
	printf("RESULT2 = %s\n", result2);

	const float data3[][2] = {
		{180.0f, 120.7f},
		{180.02f, 120.95f},
		{188.07f, 121.0},
	};
	char *result3 = polyline_encode(data3, 3);
	printf("RESULT3 = %s\n", result3);
}

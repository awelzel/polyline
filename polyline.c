#include <stdio.h>
// #include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "polyline.h"



/*
 * Encode an array of loat coordinates to a polyline. The returned
 * string is allocated and needs to be freed.
 */
static const float e5 = 100000.0f;
static const int max_5bit_chunks = 6;

static void
print_bits(const uint32_t v) {
	for (int i = 31; i >= 0; i--) {
		printf("%d", (v >> i) & 0x1);
	}
	printf("\n");
}

/*
 * Returns number of chunks created for use in copy.
 */
static int
polyline_encode_float(uint8_t *chunks, const float f)
{
	uint32_t val = fabsf(f* e5);
	int neg = (f < 0.0f);

	// 3) Two's complement for negative numbers
	//    XXX: Couldn't we just used a signed integer?
	if (neg) {
		val = ~val + 1;
	}

	// 4) Left shift by one bit...
	val <<= 1;

	// 5) Invert if original value was negative
	if (neg) {
		val = ~val;
	}

	// printf("%f val=%u neg=%d ", f, val, neg);
	// print_bits(val);

	int i = 0;
	do {
		// 5 bits from the right
		uint8_t chunk = val & 0x1f;
		val = val >> 5;

		// 8) More chunks? If so, set 0x20
		chunk |= (val > 0 ? 0x20 : 0x00);

		// 10) Add 63
		chunk += 63;

		chunks[i++] = chunk;
		// printf("chunks[%d]: (val=%u) %c ", i, val, chunk);
		// print_bits(chunk);

	} while (i < max_5bit_chunks && val > 0);

	assert(!val);
	// printf("produced %d chunks\n", i);

	return i;
}

char *
polyline_encode(const float coords[][2], size_t n)
{
	size_t allocated_size = 32;
	char *result = malloc(allocated_size);
	size_t result_idx = 0;
	const float *coords_ptr = &coords[0][0];
	float lat_prev = 0.0f, lng_prev = 0.0f;
	uint8_t chunk[max_5bit_chunks];
	for (int i = 0; i < n; i++) {
		// 1) and 2)
		float lat = coords_ptr[i * 2];
		float lng = coords_ptr[i * 2 + 1];
		// printf("lat=%f lng=%f ", lat, lng);
		lat = lat - lat_prev;
		lng = lng - lng_prev;
		// printf("diff lat=%f diff lng=%f\n", lat, lng);
		lat_prev = coords_ptr[i * 2];
		lng_prev = coords_ptr[i * 2 + 1];

		// XXX: Need to reallocate the result buffer...
		size_t chunks = polyline_encode_float(chunk, lat);
		memcpy(&result[result_idx], chunk, chunks);
		// printf("%s\n", chunk);
		result_idx += chunks;
		chunks = polyline_encode_float(chunk, lng);
		// printf("%s\n", chunk);
		memcpy(&result[result_idx], chunk, chunks);
		result_idx += chunks;
	}

	result[result_idx] = '\0';
	return result;
}


float *polyline_decode(const char *polyline)
{
	printf("polyline_decode()\n");
	uint32_t val = 0;

	int chunk_idx = 0;
	while (*polyline) {
		uint32_t chunk = *polyline;
		chunk -= 63;
		print_bits(chunk);
		val = val | ((chunk & ~0x20) << (chunk_idx * 5));
		chunk_idx++;
		printf("decode val: %20u bits:", val);
		print_bits(val);
		if (!(chunk & 0x20)) {
			printf("last chunk value!\n");
			int neg = 0;
			if (val & 0x1) {
				val = ~val;
				neg = 1;
			}
			val >>= 1;

			if (neg) {
				val = ~(val - 1);
			}

			printf("decode invert val=%u neg=%d ", val, neg);
			print_bits(val);

			chunk_idx = 0;
			val = 0;
		}
		polyline++;
	}

	return NULL;
}


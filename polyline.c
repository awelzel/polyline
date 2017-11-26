/*
 * Encode and decode Google polyline in C.
 *
 * TODO: create a dprintf macro and remove the #ifdef DEBUG stuff.
 * TODO: Check for memory allocation errors and let them bubble through.
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "polyline.h"

static const float e5 = 100000.0f;
static const int max_5bit_chunks = 6;

/* Internal structure to help keep track of allocated data for the result. */
struct buf {
	void *data;
	size_t allocs;
	size_t idx;
	size_t size; /* size as in elements, not bytes */
};

#ifdef DEBUG
static void
print_bits(const uint32_t v) {
	for (int i = 31; i >= 0; i--) {
		printf("%d", (v >> i) & 0x1);
	}
	printf("\n");
}
#endif


/*
 * Assume every coordinate just causes two bytes to be used. This will
 * allocate less memory, but more often.
 */
static int
_add_chunks_to_buf(struct buf *buf, uint8_t *chunks, size_t n, size_t coords_left) {

	if (buf->idx + n > buf->size) {
		size_t new_size = buf->size + n + coords_left * 4;
#ifdef DEBUG
		printf("encode_realloc: coords_left=%lu n=%lu idx=%lu size=%lu new_size=%lu\n",
			coords_left, n, buf->idx, buf->size, new_size);
#endif

		buf->data = realloc(buf->data, new_size * sizeof(float));
		if (!buf->data)
			return POLYLINE_NO_MEM;
		buf->size = new_size;
		buf->allocs += 1;
	}
	assert(buf->idx + n <= buf->size);
	char *data = (char *)buf->data;
	memcpy(&data[buf->idx], chunks, n);
	buf->idx += n;
    return 0;
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
/*
 * Return the length of the generated buffer.
 */
int
polyline_encode(char **polyline, const float coords[][2], size_t n)
{
	struct buf buf;
	buf.idx = 0;
	buf.allocs = 0;
	buf.data = NULL;
	buf.size = 0;

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

		size_t chunks = polyline_encode_float(chunk, lat);
		_add_chunks_to_buf(&buf, chunk, chunks, n - i);
		chunks = polyline_encode_float(chunk, lng);
		_add_chunks_to_buf(&buf, chunk, chunks, n - i);
	}
	chunk[0] = '\0';
	// Zero terminate the string. Fuck this is ugly...
	_add_chunks_to_buf(&buf, chunk, 1, 0);
	assert(!((char *)buf.data)[buf.idx - 1]);
#ifdef DEBUG
	printf("encode_buf_stats: allocs=%lu idx=%lu size=%lu strlen=%lu\n",
	       buf.allocs, buf.idx, buf.size, strlen(buf.data));
#endif
	*polyline = buf.data;
	return buf.idx - 1;
}

/*
 * Allocate some memory with the assumption that only every 6 bytes
 * result in a coordinate.
 *
 * This will err on the side of not allocating too much memory.
 */
static int
_add_coords_to_buf(struct buf *buf, const float *latlng, size_t input_left) {


	if (buf->idx >= buf->size) {
		size_t new_size = buf->size + ((input_left / max_5bit_chunks) / 2 + 1) * 2;
#ifdef DEBUG
		printf("decode_realloc: input_left=%lu idx=%lu size=%lu new_size=%lu\n",
			input_left, buf->idx, buf->size, new_size);
#endif

		buf->data = realloc(buf->data, new_size * sizeof(float));
		if (!buf->data)
			return POLYLINE_NO_MEM;

		buf->size = new_size;
		buf->allocs += 1;
	}
	assert(buf->idx + 2 <= buf->size);
	float *data = (float *)buf->data;
	data[buf->idx] = latlng[0];
	data[buf->idx + 1] = latlng[1];
	buf->idx += 2;
    return 0;
}

/*
 * Decode a polyline.
 *
 * Will allocate a buffer into `coords`. The caller is responsible to
 * free this buffer.
 *
 * Returns the number of coordinates. The number of float elements is
 * twice as big.
 */
int polyline_decode(float **coords, const char *polyline)
{
	size_t polyline_left = strlen(polyline);
	uint32_t val = 0;

	// state of allocated result!
	struct buf buf;
	buf.idx = 0;
	buf.allocs = 0;
	buf.data = NULL;
	buf.size = 0;

	// lat lng values;
	float latlng[2];
	float prev_latlng[2] = {0.0f, 0.0f};
	int latlng_idx = 0;

	int chunk_idx = 0;
	while (*polyline) {
		uint32_t chunk = *polyline++; polyline_left--;
		chunk -= 63;
		val = val | ((chunk & ~0x20) << (chunk_idx * 5));
		chunk_idx++;
		if (!(chunk & 0x20)) {
			int neg = 0;

			// Invert if originally negative
			if (val & 0x01) {
				val = ~val;
				neg = 1;
			}
			val >>= 1;

			// Reverse the two's compement operation. And
			// also make sure we properly sign extend the
			// shift that just happened (arithmetic shifting)
			if (neg) {
				val |= (1 << 31);
				val = ~(val - 1);
			}

			float f = (float)(val) / e5 * (neg ? -1.0f : 1.0f);
			latlng[latlng_idx++] = f;
			if (latlng_idx > 1) {
				prev_latlng[0] = prev_latlng[0] + latlng[0];
				prev_latlng[1] = prev_latlng[1] + latlng[1];
				_add_coords_to_buf(&buf, prev_latlng, polyline_left);
				latlng_idx = 0;
			}

			/* next round! */
			chunk_idx = 0;
			val = 0;
		}
	}
#ifdef DEBUG
	printf("decode_stats: allocs=%lu idx=%lu size=%lu\n",
		buf.allocs, buf.idx, buf.size);
#endif
	*coords = (float*)buf.data;
	return buf.idx / 2;
}

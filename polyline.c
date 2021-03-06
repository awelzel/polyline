/*
 * Encode and decode Google Polyline using C.
 */
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "polyline.h"

static const float precision = 100000.0f;
static const int max_5bit_chunks = 6;

/* Internal structure to help keep track of allocated data for the result. */
struct buf {
	void *data;
	size_t allocs;
	size_t idx;
	size_t size; /* size as in elements, not bytes */
};

#ifdef DEBUG
#define dprintf(...) do { \
	fprintf(stdout, "DEBUG polyline.%-20s -- ", __FUNCTION__); \
	fprintf(stdout, __VA_ARGS__); \
} while (0);
static void
dprint_bits(const uint32_t v) {
	for (int i = 31; i >= 0; i--) {
		fprintf(stdout, "%d", (v >> i) & 0x1);
	}
	fprintf(stdout, "\n");
}
#else
#define dprintf(...)
#define dprint_bits(...)
#endif


/*
 * Assume every coordinate just causes two bytes to be used. This will
 * allocate less memory, but more often.
 */
static int
_add_chunks_to_buf(struct buf *buf, uint8_t *chunks, size_t n, size_t coords_left) {

	dprintf("buf: size=%lu idx=%lu data=%p\n",
		buf->size, buf->idx, buf->data);
	if (buf->idx + n > buf->size) {
		size_t new_size = buf->size + n + coords_left * 4;
		dprintf("realloc: coords_left=%lu n=%lu idx=%lu size=%lu new_size=%lu\n",
			coords_left, n, buf->idx, buf->size, new_size);

		buf->data = realloc(buf->data, new_size * sizeof(float));
		if (!buf->data)
			return POLYLINE_ENOMEM;
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
_polyline_encode_float(uint8_t *chunks, const float f)
{
	/*
	 * 1) and 2) Rounding taken from python-polyline, the description
	 *           does not mention this explicitly, unfortunately :-/
	 */
	uint32_t val = (uint32_t)(floorf(fabsf(f * precision) + 0.5));
	int neg = (f < 0.0f);

	/*
	 * 3) Two's complement for negative numbers
	 *    XXX: Couldn't we maybe just use a signed integer?
	 */
	if (neg) {
		val = ~val + 1;
	}

	/* 4) Left shift by one bit... */
	val <<= 1;

	/* Invert if original value was negative */
	if (neg) {
		val = ~val;
	}

	dprintf("%f val=%u val=%#0x neg=%d ", f, val, val, neg);
	dprint_bits(val);

	int i = 0;
	do {
		/* 5 bits from the right */
		uint8_t chunk = val & 0x1f;
		val = val >> 5;

		/* 8) More chunks? If so, set 0x20 */
		chunk |= (val > 0 ? 0x20 : 0x00);

		/* 10) Add 63 */
		chunk += 63;

		/*
		dprintf("chunks[%d]: (val=%u) %c ", i, val, chunk);
		dprint_bits(chunk);
		*/

		chunks[i++] = chunk;

	} while (i < max_5bit_chunks && val > 0);

	assert(!val);

	return i;
}


int
polyline_encode(char **rptr, size_t *rsize, const float *coords, size_t n)
{
	struct buf buf = {
		.data = *rptr,
		.size = *rsize,
	};
	if (!coords || !n || (buf.data && !buf.size) || (!buf.data && buf.size))
		return POLYLINE_EINVAL;

	float lat_prev = 0.0f, lng_prev = 0.0f;
	uint8_t chunk[max_5bit_chunks];

	dprintf("start encode\n");
	for (size_t i = 0; i < n; i++) {
		float lat = coords[i * 2];
		float lng = coords[i * 2 + 1];

		lat = lat - lat_prev;
		lng = lng - lng_prev;

		lat_prev = coords[i * 2];
		lng_prev = coords[i * 2 + 1];

		size_t chunks = _polyline_encode_float(chunk, lat);
		if (_add_chunks_to_buf(&buf, chunk, chunks, n - i)) {
			return POLYLINE_ENOMEM;
		}
		chunks = _polyline_encode_float(chunk, lng);
		if (_add_chunks_to_buf(&buf, chunk, chunks, n - i)) {
			return POLYLINE_ENOMEM;
		}
	}
	chunk[0] = '\0';
	/*
	 * Zero terminate the string. Fuck this is ugly, but we may
	 * need to extend the buffer size by one more...
	 *
	 * XXX: Maybe we should just always have at least one byte left...
	 */
	_add_chunks_to_buf(&buf, chunk, 1, 0);
	assert(!((char *)buf.data)[buf.idx - 1]);
	dprintf("encode buf stats: allocs=%lu idx=%lu size=%lu strlen=%lu\n",
	        buf.allocs, buf.idx, buf.size, strlen(buf.data));
	*rptr = buf.data;
	*rsize = buf.size;
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
		dprintf("realloc: input_left=%lu idx=%lu new_size=%lu "
			"buf->size=%lu buf->data=%p\n",
			input_left, buf->idx, new_size, buf->size, buf->data);

		buf->data = realloc(buf->data, new_size * sizeof(float));
		if (!buf->data)
			return POLYLINE_ENOMEM;

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

int
polyline_decode(float **const rptr, size_t *rsize, const char *polyline)
{
	struct buf buf = {
		.data = *rptr,
		.size = *rsize,
	};
	uint32_t val = 0;
	size_t polyline_left = 0;
	float latlng[2], prev_latlng[2] = {0.0f, 0.0f};
	int latlng_idx = 0, chunk_idx = 0;

	if (!polyline || (buf.data && !buf.size) || (!buf.data && buf.size))
		return POLYLINE_EINVAL;

	polyline_left = strlen(polyline);
	dprintf("start decode buf.size=%lu buf.data=%p polyline_left=%lu\n",
			buf.size, buf.data, polyline_left);
	while (*polyline) {
		uint32_t chunk = *polyline++; polyline_left--;
		dprintf("decode chunk: '%c'\n", chunk);
		/*
		 * 0001 1111    0x1f max input bits)
		 * 0011 1111    0x3f max value with continuation marker: '?'
		 * 		This is the minimum character required as well.
		 * 0011 1111    0x3f added to the max value above: '?'
		 * 0111 1110    0x7e maximum allowed character in polyline: '~'
		 *
		 * Continuation characters set:
		 * _`abcdefghijklmnopqrstuvwxyz{|}
		 *
		 * Terminal characters set:
		 * ?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^
		 */
		if (chunk < 0x3f || chunk > 0x7e) {
			*rptr = buf.data;
			*rsize = buf.size;
			return POLYLINE_EPARSE;
		}
		chunk -= 0x3f;
		val = val | ((chunk & ~0x20) << (chunk_idx * 5));
		chunk_idx++;

		if (!(chunk & 0x20)) {
			int neg = 0;

			/* Invert if originally negative. */
			if (val & 0x01) {
				val = ~val;
				neg = 1;
			}
			val >>= 1;

			/*
			 * Reverse the two's compement operation. And
			 * also make sure we properly sign extend the
			 * shift that just happened (arithmetic shifting)
			 */
			if (neg) {
				val |= (1 << 31);
				val = ~(val - 1);
			}

			float f = (float)(val) / precision * (neg ? -1.0f : 1.0f);
			latlng[latlng_idx++] = f;
			if (latlng_idx > 1) {
				prev_latlng[0] = prev_latlng[0] + latlng[0];
				prev_latlng[1] = prev_latlng[1] + latlng[1];
				if (_add_coords_to_buf(&buf, prev_latlng, polyline_left)) {
					*rptr = NULL;
					*rsize = 0;
					return POLYLINE_ENOMEM;
				}
				latlng_idx = 0;
			}

			/* next round! */
			chunk_idx = 0;
			val = 0;
		}
	}

	if (val > 0 || latlng_idx > 0 || chunk_idx > 0) {
		*rptr = buf.data;
		*rsize = buf.size;
		return POLYLINE_ETRUNC;
	}
	dprintf("decode buf stats: allocs=%lu idx=%lu size=%lu\n",
		buf.allocs, buf.idx, buf.size);
	*rptr = buf.data;
	*rsize = buf.size;
	return buf.idx / 2;
}


/* This needs to be kept in nice order! */
static const char *error_map[] = {
	NULL,
	"POLYLINE_ENOMEM", /* -1 Return value if a memory allocation failed. */
	"POLYLINE_EINVAL", /* -2 Invalid arguments. */
	"POLYLINE_EPARSE", /* -3 Failed to decode a polyline. */
	"POLYLINE_ETRUNC", /* -4 Truncated polyline during decode. */
	"POLYLINE_ERANGE", /* -5 Coordinates out of range. Allowed is -180.0 to 180.0 */
};

const char *
polyline_strerror(int error)
{
	dprintf("error_map entries: %lu\n", sizeof(error_map) / sizeof(error_map[0]));
	if (error > 0)
		return NULL;

	error = -error;
	if (error >= (int)(sizeof(error_map) / sizeof(error_map[0])))
		return NULL;

	return error_map[error];
}

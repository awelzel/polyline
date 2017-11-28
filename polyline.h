/**
 * @file
 * Google Polyline encoding and decoding in C.
 *
 * https://developers.google.com/maps/documentation/utilities/polylinealgorithm
 */
#ifndef __POLYLINE_H__
#define __POLYLINE_H__
#include <stdlib.h>

#define POLYLINE_ENOMEM -1 /**< Return value if a memory allocation failed. */
#define POLYLINE_EINVAL -2 /**< Invalid arguments. */
#define POLYLINE_EPARSE -3 /**< Failed to decode a polyline. */
#define POLYLINE_ETRUNC -4 /**< Truncated polyline during decode. */
#define POLYLINE_ERANGE -5 /**< Coordinates out of range. Allowed is -180.0 to 180.0 */

/**
 * Encode an array of floats to a Google Polyline string.
 *
 * @param rptr Pointer to a `char*` which will be assigned an
 * 	allocated C string. The result is guaranteed to be
 * 	null byte (`\0') terminated.
 * @param rsize Size of array provided or allocated.
 * @param coords Pointer to an array of float elements representing the
 * 		  coordinates to be encoded. As each coordinate consists
 * 		  of two `float` elements, the number of
 * 		  `float` elements in this array must be even.
 * @param n Number of coordinates to be encoded. Note, the size of
 *          the `coords` array must be `2 * n`.
 *
 * @return On success, returns the length (`strlen()`) of the C string
 *         assigned to `polyline`.
 * 	   On error, a value < 0 is returned.
 */
int polyline_encode(char **rptr, size_t *rsize, const float *coords, size_t n);

/**
 * Decode a Google Polyline string to an array of floats.
 *
 * The result is stored in a large enough buffer and returned
 * to the caller by means of the `rdst` parameter.
 * This pointer, if not NULL, should be passed to `free()` to release
 * the allocated storage when it is no longer needed.
 *
 * `rptr` and `size` may point to an existing buffer, in which case it
 * will be reused. If set to NULL and 0, respectively, a new buffer
 * will be allocated. Compare with `getline(3)` behavior.
 *
 * @param rptr Pointer to an array of floats for the result.
 * @param rsize Size of array provided or allocated.
 * @param polyline C string representing a Google Polyline. Must be
 * 	null byte (`\0`) terminated.
 *
 * @return On success, returns the number of *coordinates*. Note, the
 * 	number of float elements in the buffer will be twice the
 * 	return value. On error, a value < 0 is returned. If `*rptr` is
 * 	not `NULL` on error, the user should release it using `free()`.
 */
int polyline_decode(float **rptr, size_t *rsize, const char *polyline);

/**
 * Return a pointer to a string that describes the error code.
 *
 * @param error Error code as returned by @ref polyline_encode() or
 * 	@ref polyline_decode().
 *
 * @return Pointer to a string describing the error code, or NULL if
 * 	the error code does not exist.
 */
const char *polyline_strerror(int error);
#endif

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
#define POLYLINE_ERANGE -4 /**< Coordinates out of range. Allowed is -180.0 to 180.0 */

/**
 * Encode an array of floats to a Google Polyline string.
 *
 * @param polyline Pointer to a `char*` which will be assigned an
 * 		   allocated C string. The result is guaranteed to be
 * 		   null byte (`\0') terminated.
 * @param coords Pointer to an array of float elements representing the
 * 		  coordinates to be encoded. As each coordinate consists
 * 		  of two `float` elements, the number of
 * 		  `float` elements in this array must be even.
 * @param n Number of coordinates to be encoded. Note, the size of
 *          the `coords` array must be `2 * n`.
 *
 * @return On success, returns the length (`strlen()`) of the C string
 *         assigned to `polyline`.
 * 	   On error, a value < 0 is returned. Possible error
 * 	   values include @ref POLYLINE_OOM.
 */
int polyline_encode(char **dst, size_t *size, const float *coords, size_t n);

/**
 * Decode a Google Polyline string to an array of floats.
 *
 * The result is stored in a large enough buffer and returned
 * to the caller by means of the `coords` parameter.
 * This pointer should be passed to `free()` in order to release the
 * allocated storage when it is no longer needed.
 *
 * @param dst Pointer to a `float *` into which the allocated buffer
 * 	will be stored.
 * @param polyline C string representing a Google Polyline. Must be
 * 	null byte (`\0`) terminated.
 *
 * @return On success, returns the number of *coordinates*. Note, the
 * 	number of float elements in the buffer will be twice the
 * 	return value. On error, a value < 0 is returned. Possible error
 * values include @ref POLYLINE_OOM.
 */
int polyline_decode(float **dst, size_t *size, const char *polyline);

#endif

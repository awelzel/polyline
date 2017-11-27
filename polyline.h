#ifndef __POLYLINE_H__
#define __POLYLINE_H__

#include <stdlib.h>

#define POLYLINE_NO_MEM -5

/**
 * Encode and array of floats to a Google Polyline string.
 */
int polyline_encode(char **polyline, const float const *coords, size_t n);

/**
 * Decode a Google Polyline string to an array of floats.
 */
int polyline_decode(float **coords, const char *const polyline);

#endif

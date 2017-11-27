#ifndef __POLYLINE_H__
#define __POLYLINE_H__

char * polyline_encode(const float coords[][2], size_t n);
float * polyline_decode(const char *const polyline);

#endif

#ifndef __POLYLINE_H__
#define __POLYLINE_H__

#define POLYLINE_NO_MEM -5

int polyline_encode(char **polyline, const float coords[][2], size_t n);
int polyline_decode(float **coords, const char *const polyline);

#endif

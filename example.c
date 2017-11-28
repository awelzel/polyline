#include <stdio.h>
#include "polyline.h"

int main()
{
	float *fptr = NULL; size_t fsize = 0;
	char *cptr = NULL; size_t csize = 0;
	int r = polyline_decode(&fptr, &fsize, "_p~iF~ps|U_ulLnnqC_mqNxxq`@");
	for (int i = 0; i < r; i++)
		printf("(%f, %f)%s", fptr[i * 2], fptr[i * 2 + 1], i < (r - 1) ? " " : "\n");

	polyline_encode(&cptr, &csize, fptr, r);
	printf("%s\n", cptr);
	free(fptr);
	free(cptr);
}

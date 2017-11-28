# polyline
Google Polyline encoding and decoding with C.

## Compiling and basic library usage (without error handling)

    $ make
    ...
    $ cat example.c
    #include <stdio.h>
    #include "polyline.h"
    
    int main(int argc, char *argv[])
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
    $ gcc -o example example.c libpolyline.a -lm
    $ ./example
    (38.500000, -120.199997) (40.700001, -120.949997) (43.251999, -126.453003)
    _p~iF~ps|U_ulLnnqC_mqNxxq`@


## Command-line usage

A simple command-line utility is included.

### Decoding

    $ ./polyline '_p~iF~ps|U_ulLnnqC_mqNxxq`@'
    [[38.50000, -120.20000], [40.70000, -120.95000], [43.25200, -126.45300]]

    $ echo '_p~iF~ps|U_ulLnnqC_mqNxxq`@' | ./polyline -d
    [[38.50000, -120.20000], [40.70000, -120.95000], [43.25200, -126.45300]]

    $ ./polyline -p 1 '_p~iF~ps|U_ulLnnqC_mqNxxq`@'
    [[38.5, -120.2], [40.7, -120.9], [43.3, -126.5]]


### Encoding

    $ polyline -e '[[38.50000, -120.20000], [40.70000, -120.95000], [43.25200, -126.45300]]'
    _p~iF~ps|U_ulLnnqC_mqNxxq`@
    $ echo '38.5 -120.2 40.7 -120.95 43.252 -126.453'| ./polyline -e
    _p~iF~ps|U_ulLnnqC_mqNxxq`@

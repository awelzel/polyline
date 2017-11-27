/*
 * Polyline command line utility.
 */
#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>

#include "polyline.h"

#define eprintf(...) fprintf(stderr, __VA_ARGS__)

static char* program = NULL;


static void
decode_line(float **dst, size_t *size, const char *line, int precision) {
	int r;
	if ((r = polyline_decode(dst, size, line)) < 0) {
		eprintf("Failed to decode '%s'", line);
		return;
	}

	printf("[");
	for (int i = 0; i < r; i++) {
		printf("[%.*f, %.*f]%s",
		       precision, (*dst)[i * 2],
		       precision, (*dst)[i * 2 + 1],
		       (i < (r - 1)) ? ", " : ""
		);
	}
	printf("]\n");
}

/* Replace these input characters with spaces when encoding. */
static char *replace_chars = "[]{}(),";


/* Read one float from nptr with the same symantics as
 * strtof(), but store the result in ftpr.
 *
 * @return -1 on error and 1 when done.
 */
static int
_strtof(float *fptr, char *nptr, char **endptr)
{
	*fptr = strtof(nptr, endptr);
	if (nptr == *endptr) {
		return 1;
	}

	/* endptr pointing to a non-whitespace character? */
	if (**endptr && **endptr != ' ') {
		eprintf("endptr %c\n", **endptr);
		return -1;
	}

	return 0;
}

static int
encode_line(char **dst, size_t *size, char *line) {
	int i, r, floats = 0, in_space = 1;
	char *ptr = line, *endptr;
	float *latlngs;

	/*
	 * Replace characters. '[(1.0, 2.0)]'ends up as '  1.0  2.0  '
	 * so we can more easily parse it...
	 */
	while (*ptr) {
		for (size_t i = 0; i < sizeof(replace_chars); i++)
			if (*ptr == replace_chars[i])
				*ptr = ' ';
		if (*ptr != ' ') {
			if (in_space) {
				floats += 1;
				in_space = 0;
			}
		} else {
			in_space = 1;
		}
		ptr++;
	}

	if (floats & 1) {
		eprintf("odd number of floats in line: %d", floats);
		printf("\n");
		return 0;
	}
	latlngs = malloc(floats * sizeof(float));
	if (!latlngs) {
		eprintf("%s: out of memory!", program);
		return -1;
	}

	/* Reset to the beginning. */
	ptr = line;
	i = 0;
	while (*ptr && i < floats) {
		r = _strtof(&latlngs[i], ptr, &endptr);
		if (r < 0) {
			eprintf("invalid decimal number starting at: '%s'\n", endptr);
			printf("\n"); /* empty line */
			goto err;
		}
		ptr = endptr;
		i++;
	}
	assert(floats == i);

	if ((r = polyline_encode(dst, size, latlngs, i / 2)) < 0) {
		eprintf("Failed to encode '%s'", line);
		printf("\n"); /* Empty line on errors */
		goto err;
	}
	printf("%s\n", *dst);
err:
	free(latlngs);
	return 0;
}

static void
usage()
{
	eprintf("Usage: %s [-h|-d|-e] [polyline]\n\n", program);
	eprintf("Options:\n");
	eprintf("  -h             Display this help message.\n");
	eprintf("  -d [default]   Decode a polyline.\n");
	eprintf("  -e             Encode coordinates and output a polyline.\n");
	eprintf("  -p [default 5] Output precision when decoding. 0 to 10.\n");
	eprintf("\n"
	        "If no argument is provided following the options input\n"
		"will be read from stdin.\n");
}


int
main(int argc, char *argv[])
{
	program = argv[0];
	int opt, encode = 0, decode = 0;
	int precision = 5;
	char *endptr;

	void *dst = NULL;
	size_t dst_size = 0;

	opterr = 1;
	while ((opt = getopt(argc, argv, "dehp:")) >= 0) {
		switch(opt) {
		case 'e':
			encode = 1;
			break;
		case 'd':
			decode = 1;
			break;
		case 'p':
			precision = strtol(optarg, &endptr, 10);
			if (*endptr || precision < 0 || precision > 10) {
				eprintf("%s: invalid precision -- '%s'\n",
					argv[0], optarg);
				return 1;
			}
			break;
		case 'h':
			usage();
			return 0;
		case '?': /* Unknown option */
			return 1;
		}
	}
	if (encode && decode) {
		eprintf("%s: specifying -e and -d is invalid\n", argv[0]);
		return 1;
	}
	/* Default to decode if nothing was set. */
	decode = (!encode && !decode) || decode;

	if (optind == argc) {
		char *lineptr = NULL;
		size_t n = 0;
		int r;
		while ((r = getline(&lineptr, &n, stdin)) >= 0) {
			if (lineptr[r - 1] == '\n') {
				lineptr[r - 1] = '\0';
			}

			if (decode) {
				decode_line((float **)&dst, &dst_size, lineptr, precision);
			} else {
				encode_line((char **)&dst, &dst_size, lineptr);
			}
		}
		if (lineptr && n > 0)
			free(lineptr);
	} else {
		if (argc > optind + 1) {
			eprintf("%s: too many arguments\n", program);
			return 1;
		}
		if (decode) {
			decode_line((float **)&dst, &dst_size, argv[optind], precision);
		} else {
			encode_line((char **)&dst, &dst_size, argv[optind]);
		}
	}

	if (dst)
		free(dst);
}

.PHONY: clean

CFLAGS ?= -O2 -Wall -pedantic -ansi -std=c99
LIBS = -lm
OBJS = test.o polyline.o
BINS = test
test: test.o polyline.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	$(RM) $(OBJS) $(BINS)

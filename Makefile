.PHONY: clean

CFLAGS=-O3 -Wall -pedantic -ansi -std=c99
OBJS = test.o polyline.o
BINS = test
test: test.o polyline.o

clean:
	$(RM) $(OBJS) $(BINS)


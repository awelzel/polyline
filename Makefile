.PHONY: clean

CFLAGS ?= -O2 -Wall -pedantic -ansi -std=c99
LIBS = -lm
BINS = test polyline

all: test libpolyline.a polyline

main.o: main.c polyline.h
polyline.o: polyline.c polyline.h
test.o: test.c polyline.h

.o: polyline.h

test: test.o polyline.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

libpolyline.a: polyline.o
	$(AR) rcs $@ $^

polyline: main.o polyline.h libpolyline.a
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	$(RM) *.o libpolyline.a $(BINS)

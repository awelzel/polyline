.PHONY: clean

CFLAGS ?= -O2 -Wextra -pedantic -std=gnu11
LIBS = -lm
BINS = test polyline

all: test libpolyline.a polyline

polyline.o: polyline.c polyline.h
	$(CC) $(CFLAGS) -Wextra -c $<

main.o: main.c polyline.h
test.o: test.c polyline.h

test: test.o polyline.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

libpolyline.a: polyline.o
	$(AR) rcs $@ $^

polyline: main.o polyline.h libpolyline.a
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBS)

clean:
	$(RM) *.o libpolyline.a $(BINS)

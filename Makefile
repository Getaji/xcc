CFLAGS=-std=c11 -g -static
DEPENDENCIES=$(wildcard *.c)
SRCS=$(filter-out samplefn.c, $(wildcard *.c))
OBJS=$(SRCS:.c=.o)

xcc: $(DEPENDENCIES)
	$(CC) -c samplefn.c
	$(CC) -o xcc $(OBJS) $(LDFLAGS)

$(OBJS): xcc.h

test: xcc
	./test.sh

clean:
	rm -f xcc *.o *~ tmp*

.PHONY: test clean

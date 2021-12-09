CFLAGS=-std=c11 -g -static
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

xcc: $(OBJS)
	$(CC) -o xcc $(OBJS) $(LDFLAGS)

$(OBJS): xcc.h

test: xcc
	./test.sh

clean:
	rm -f xcc *.o *~ tmp*

.PHONY: test clean

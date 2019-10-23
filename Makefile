#
#   Copyrigin(c) 2019 firesidefox. All rights reserved.
#

CFLAG := -g -O0

# make build to build libredisae[.a]
.PHONY: static clean

libredisaeobjs=ae.o zmalloc.o

all: udpecho

udpecho: static udpecho.c
	gcc $(CFLAG) udpecho.c -L. -lredisae -o $@

static: libredisae.a

libredisae.a: $(libredisaeobjs)
	ar crv $@ $^

$(libredisaeobjs): %.o: %.c
	gcc $(CFLAG) -c $^ -o $@

clean:
	rm -rf $(libredisaeobjs) echoserver udpecho libredisae.a
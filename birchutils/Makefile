flags=-O3 -Wall -std=c2x -I.
ldflags=-fPIC -shared -ldl -D_GNU_SOURCE
libdir=/lib/x86_64-linux-gnu/
incdir=/usr/include

all: clean birchutils.so

install: birchutils.so
	cp birchutils.so $(libdir)/libbu.so
	cp birchutils.h  $(incdir)
	ldconfig

birchutils.so: birchutils.o
	cc ${flags} $^ -o $@ ${ldflags}

birchutils.o: birchutils.c
	cc ${flags} -c $^ $(ldflags)

clean:
	rm -f birchutils.o birchutils.so

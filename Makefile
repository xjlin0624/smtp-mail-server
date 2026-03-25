flags=-O2 -Wall -std=c2x
# ldflags=-lbu
ldflags=

.PHONY: all clean

all: pigeon

pigeon.o: pigeon.c pigeon.h
	cc $(flags) -c $<

pigeon: pigeon.o client.o birchutils/birchutils.o
	cc $(flags) $^ -o $@ $(ldflags)

# client: pigeon.o client.o birchutils/birchutils.o
# 	cc $(flags) $^ -o $@ $(ldflags)
 
client.o: client.c pigeon.h
	cc $(flags) -c $<

birchutils/birchutils.o: birchutils/birchutils.c birchutils/birchutils.h
	cc $(flags) -c $< -o $@

clean:
	rm -f *.o birchutils/*.o pigeon client


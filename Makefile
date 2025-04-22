PREFIX ?= /usr/local/
PROGRAM=jonark-fetcher
CC=gcc
CFLAGS=-I$(PREFIX)/include
LDFLAGS=-L$(PREFIX)/lib -lcollectc

objects=main.o

all: $(PROGRAM)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(PROGRAM): $(objects)
	$(CC) $^ $(CFLAGS) -o $@ $(LDFLAGS)

clean:
	rm -f $(PROGRAM) $(objects)

.PHONY: all $(PROGRAM) clean

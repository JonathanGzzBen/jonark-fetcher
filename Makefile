PREFIX ?= /usr/local
PROGRAM := jonark-fetcher
CC := gcc


CFLAGS ?= -g
CFLAGS += -I$(PREFIX)/include
WARNINGS ?= -Wall -Wextra

LDFLAGS ?=
LDFLAGS += -L$(PREFIX)/lib
LDLIBS := -lcollectc

SRCS := main.c
OBJS := $(SRCS:.c=.o)

all: $(PROGRAM)

%.o: %.c
	$(CC) $(CFLAGS) $(WARNINGS) -c $< -o $@

$(PROGRAM): $(OBJS)
	$(CC) $^ $(CFLAGS) -o $@ $(LDFLAGS) $(LDLIBS)

clean:
	$(RM) $(PROGRAM) $(OBJS)

.PHONY: all $(PROGRAM) clean

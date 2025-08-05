CC=gcc
CFLAGS=-Wall -Wextra -std=c99 -Iinclude
LIBS=-lsqlite3

# Source and build directories
SRCDIR=src
BUILDDIR=build
INCLUDEDIR=include

# Source files
SOURCES=$(wildcard $(SRCDIR)/*.c)
OBJECTS=$(SOURCES:$(SRCDIR)/%.c=$(BUILDDIR)/%.o)

all: $(BUILDDIR)/causaldb

$(BUILDDIR)/causaldb: $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

benchmark: $(BUILDDIR)/benchmark

$(BUILDDIR)/benchmark: benchmarks/benchmark.c $(SRCDIR)/db.c $(SRCDIR)/event.c
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -o $@ $^ $(LIBS)

clean:
	rm -rf $(BUILDDIR)/*
	cd server && make clean

.PHONY: all clean benchmark
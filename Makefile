CXX ?= g++
CC ?= gcc

CFLAGS = -Ofast -std=c++1y -Wall -Werror -g
LDFLAGS = -lpapi
MALLOC_LDFLAGS = -ldl

all: bench_tmp

malloc_count.o: malloc_count/malloc_count.c  malloc_count/malloc_count.h
	$(CC) -O2 -Wall -Werror -g -c -o $@ $<

bench_tmp: bench_tmp.cpp *.h */*.h
	$(CXX) $(CFLAGS) -o $@ $< $(LDFLAGS)

bench_tmp_malloc: bench_tmp.cpp *.h */*.h malloc_count.o
	$(CXX) $(CFLAGS) -DMALLOC_INSTR -o $@ $< malloc_count.o $(LDFLAGS) $(MALLOC_LDFLAGS)

run: bench_tmp
	./bench_tmp

run_malloc: bench_tmp_malloc
	./bench_tmp_malloc

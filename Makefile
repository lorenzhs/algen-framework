CXX ?= g++
CC ?= gcc

CFLAGS = -Ofast -std=c++1y -Wall -Werror -g
LDFLAGS = -lpapi -ldl

all: bench_tmp

malloc_count.o: malloc_count/malloc_count.c  malloc_count/malloc_count.h
	$(CC) -O2 -Wall -Werror -g -c -o $@ $<

bench_tmp: bench_tmp.cpp *.h */*.h malloc_count.o
	$(CXX) $(CFLAGS) -o $@ $< malloc_count.o $(LDFLAGS)

run: bench_tmp
	./bench_tmp

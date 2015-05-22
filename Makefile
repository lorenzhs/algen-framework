CXX ?= g++

CFLAGS = -Ofast -std=c++1y -Wall -Werror -g
LDFLAGS = -lpapi

bench_tmp: bench_tmp.cpp *.h */*.h
	$(CXX) $(CFLAGS) -o $@ $< $(LDFLAGS)

run: bench_tmp
	./bench_tmp

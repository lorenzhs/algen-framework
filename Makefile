# g++ >= 4.9 or clang++ >= 3.4 is required to build this.
# clang++ 3.4 and 3.5 cannot build the compare target with debug information.
# Either remove "-g" from the flags or use clang++ >= 3.6 (or g++).
CX ?= clang++-3.6
CC ?= gcc

SANITIZER ?= address

COMMONFLAGS = -std=c++1y -Wall -Wextra -Werror
CFLAGS = ${COMMONFLAGS} -Ofast -g -DNDEBUG
DEBUGFLAGS = ${COMMONFLAGS} -O0 -ggdb3
LDFLAGS = -lpapi -lboost_serialization
MALLOC_LDFLAGS = -ldl

all: bench_hash bench_pq

clean:
	rm -f *.o bench_hash bench_hash_malloc debug sanitize

malloc_count.o: malloc_count/malloc_count.c  malloc_count/malloc_count.h
	$(CC) -O2 -Wall -Werror -g -c -o $@ $<

bench_hash: bench_hash.cpp */*.h
	$(CX) $(CFLAGS) -o $@ $< $(LDFLAGS)

bench_hash_malloc: bench_hash.cpp */*.h malloc_count.o
	$(CX) $(CFLAGS) -DMALLOC_INSTR -o $@ $< malloc_count.o $(LDFLAGS) $(MALLOC_LDFLAGS)

bench_pq: bench_pq.cpp */*.h
	$(CX) $(CFLAGS) -o $@ $< $(LDFLAGS)

bench_pq_malloc: bench_pq.cpp */*.h malloc_count.o
	$(CX) $(CFLAGS) -DMALLOC_INSTR -o $@ $< malloc_count.o $(LDFLAGS) $(MALLOC_LDFLAGS)

debug_hash: bench_hash.cpp */*.h
	$(CX) $(DEBUGFLAGS) -o $@ $< $(LDFLAGS)

debug_hash_malloc: bench_hash.cpp */*.h malloc_count.o
	$(CX) $(DEBUGFLAGS) -DMALLOC_INSTR -o $@ $< malloc_count.o $(LDFLAGS) $(MALLOC_LDFLAGS)

debug_pq: bench_pq.cpp */*.h
	$(CX) $(DEBUGFLAGS) -o $@ $< $(LDFLAGS)

debug_pq_malloc: bench_pq.cpp */*.h malloc_count.o
	$(CX) $(DEBUGFLAGS) -DMALLOC_INSTR -o $@ $< malloc_count.o $(LDFLAGS) $(MALLOC_LDFLAGS)

sanitize_hash: bench_hash.cpp */*.h
	$(CX) $(CFLAGS) -fsanitize=${SANITIZER} -o $@ $< $(LDFLAGS)
	./$@

sanitize_pq: bench_pq.cpp */*.h
	$(CX) $(CFLAGS) -fsanitize=${SANITIZER} -o $@ $< $(LDFLAGS)
	./$@

compare: compare.cpp */*.h
	$(CX) $(CFLAGS) -o $@ $< $(LDFLAGS)

run_hash: bench_hash
	./bench_hash

run_hash_malloc: bench_hash_malloc
	./bench_hash_malloc

run_pq: bench_pq
	./bench_pq

run_pq_malloc: bench_pq_malloc
	./bench_pq_malloc

#pragma once

#include <type_traits>
#include <vector>

#include "../common/benchmark.h"
#include "../common/contenders.h"
#include "../common/timer.h"

namespace pq {

template <typename PQ>
class microbenchmark {
public:
	using Benchmark = common::benchmark<PQ, size_t>;
	using BenchmarkFactory = common::contender_factory<Benchmark>;
	common::contender_list<Benchmark> benchmarks;

	static void register_benchmarks(common::contender_list<Benchmark> &benchmarks) {
		auto fill = [](PQ &queue, size_t num) {
			for (size_t i = 1; i <= num; ++i) {
				queue.push(i);
			}
		};

		const std::vector<size_t> sizes{1<<16, 1<<18, 1<<20, /*1<<22, 1<<24, 1<<26*/};

		common::register_benchmark("insert", "insert",  fill, sizes, benchmarks);

		common::register_benchmark("pop", "pop", fill, [](PQ &queue, size_t num) {
			for (size_t i = 0; i < num; ++i) {
				queue.pop();
			}		
		}, sizes, benchmarks);
	}
};
}

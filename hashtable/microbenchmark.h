#pragma once

#include <type_traits>

#include "../common/timer.h"

namespace hashtable {

template <typename HashTable>
class microbenchmark {
public:
	using Benchmark = common::benchmark<HashTable, size_t>;
	using BenchmarkFactory = common::contender_factory<Benchmark>;
	common::contender_list<Benchmark> benchmarks;

	static void register_benchmarks(common::contender_list<Benchmark> &benchmarks) {
		auto fill = [](HashTable &map, size_t num) {
			for (size_t i = 1; i <= num; ++i) {
				map[i] = num - i;
			}
		};

		const std::vector<size_t> sizes{1<<16, 1<<18, 1<<20, /*1<<22, 1<<24, 1<<26*/};

		common::register_benchmark("insert", "insert",  fill, sizes, benchmarks);

		common::register_benchmark("insert+find", "insert-find", [](HashTable &map, size_t num) {
			for (size_t i = 1; i <= num; ++i) {
				map[i] = i; // TODO randomize
			}
			for (size_t i = 1; i <= num; ++i) {
				map.find(i); // TODO randomize
			}
		}, sizes, benchmarks);


		common::register_benchmark("ins-del-ins + del-ins-del", "ins-del-cycle", [](HashTable &map, size_t num) {
			for (size_t i = 1; i <= num; ++i) {
				// TODO randomize
				map[i] = i;
				map.erase(i);
				map[i] = num-i;
			}
			for (size_t i = 1; i <= num; ++i) {
				map.erase(i);
				map[i] = i;
				map.erase(i);
			}
		}, sizes, benchmarks);

		common::register_benchmark("access", "access", fill, [](HashTable &map, size_t num) {
			for (size_t i = 1; i <= num; ++i) {
				(void)map[i];
			}
		}, sizes, benchmarks);

		common::register_benchmark("find", "find", fill, [](HashTable &map, size_t num) {
			for (size_t i = 1; i <= num; ++i) {
				(void)map.find(i);
			}
		}, sizes, benchmarks);
	}
};
}

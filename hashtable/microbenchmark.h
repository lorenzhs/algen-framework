#pragma once

#include "../common/timer.h"

namespace hashtable {

template <typename HashTable>
class microbenchmark {
public:
	using Benchmark = common::benchmark<HashTable>;
	using BenchmarkFactory = common::contender_factory<Benchmark>;
	common::contender_list<Benchmark> benchmarks;

	static void register_benchmarks(common::contender_list<Benchmark> &benchmarks) {
		auto fill = [](HashTable &&map) {
			const int num = 1000000;
			for (size_t i = 0; i < num; ++i) {
				map[i] = num - i;
			}
		};

		common::register_benchmark("insert", "insert",  fill, benchmarks);

		common::register_benchmark("insert+find", "insert-find", [](HashTable &&map) {
			const int num = 1000000;
			for (int i = 0; i < num; ++i) {
				map[i] = i; // TODO randomize
			}
			int sum = 0;
			for (int i = 0; i < num; ++i) {
				sum += map.find(i); // TODO randomize
			}
			return sum;
		}, benchmarks);


		common::register_benchmark("ins-del-ins + del-ins-del", "ins-del-cycle", [](HashTable &&map) {
			const int num = 1000000;
			for (size_t i = 0; i < num; ++i) {
				// TODO randomize
				map[i] = i;
				map.erase(i);
				map[i] = num-i;
			}
			for (size_t i = 0; i < num; ++i) {
				map.erase(i);
				map[i] = i;
				map.erase(i);
			}
		}, benchmarks);

		common::register_benchmark("access", "access", fill, [](HashTable &&map) {
			const int num = 1000000;
			for (size_t i = 0; i < num; ++i) {
				(void)map[i];
			}
		}, benchmarks);

		common::register_benchmark("find", "find", fill, [](HashTable &&map) {
			const int num = 1000000;
			for (size_t i = 0; i < num; ++i) {
				(void)map.find(i);
			}
		}, benchmarks);
	}
};
}

#include <iostream>
#include <vector>

#include <papi.h>

#include "common/benchmark.h"
#include "common/contenders.h"
#include "common/instrumentation.h"
#include "hashtable/unordered_map.h"
#include "hashtable/microbenchmark.h"

int main(void) {
	using HashTable = hashtable::hashtable<int, int>;
	using Benchmark = common::benchmark<HashTable>;

	common::contender_list<HashTable> contenders;
	hashtable::unordered_map<int, int>::register_contenders(contenders);

	common::contender_list<Benchmark> benchmarks;
	hashtable::microbenchmark<HashTable>::register_benchmarks(benchmarks);

	//std::vector<std::function<common::instrumentation*(void)>> instrumentations = {
	//	[] { return new common::timer_instrumentation(); },
	//	[] { return new common::papi_instrumentation_cache(); },
	//	[] { return new common::papi_instrumentation_instr(); }};

	common::contender_list<common::instrumentation> instrumentations;
	instrumentations.register_contender("timer",
		[](){ return new common::timer_instrumentation(); },
		[](common::instrumentation* instr) { delete (common::timer_instrumentation*) instr; });

	instrumentations.register_contender("PAPI cache",
		[](){ return new common::papi_instrumentation_cache(); },
		[](common::instrumentation* instr) { delete (common::papi_instrumentation_cache*) instr; });

	for (auto instrumentation : instrumentations) {
		std::cout << "Running benchmark with " << instrumentation.description() << " instrumentation" << std::endl;
		std::vector<common::benchmark_result*> results;
		for (auto datastructure_factory : contenders) {
			for (auto benchmark_factory : benchmarks) {
				auto benchmark = benchmark_factory();
				auto t = benchmark->run(datastructure_factory, instrumentation);
				std::cout << "Data Structure: " << datastructure_factory.description() << "; "
						  << "Benchmark: " << benchmark_factory.description() << "; "
						  << "Result: " << *t << std::endl;
				delete benchmark;
				results.push_back(t);
			}
		}
		// Delete results
		auto temp = instrumentation();
		temp->destroy(results.begin(), results.end());
		instrumentation.destroy(temp);
		results.clear();
		std::cout << std::endl << std::endl;
	}

	// Shut down PAPI if it was used
	if (PAPI_is_initialized() != PAPI_NOT_INITED)
		PAPI_shutdown();
}

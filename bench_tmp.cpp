#include <fstream>
#include <iostream>
#include <vector>

#include <papi.h>

#include "common/arg_parser.h"
#include "common/benchmark.h"
#include "common/contenders.h"
#include "common/instrumentation.h"
#include "hashtable/unordered_map.h"
#include "hashtable/microbenchmark.h"

int main(int argc, char** argv) {
	common::arg_parser args(argc, argv);
	std::string resultfn = args.get<std::string>("o", "results.txt");

	using HashTable = hashtable::hashtable<int, int>;
	using Benchmark = common::benchmark<HashTable>;

	// Set up data structure contenders
	common::contender_list<HashTable> contenders;
	hashtable::unordered_map<int, int>::register_contenders(contenders);

	// Register Benchmarks
	common::contender_list<Benchmark> benchmarks;
	hashtable::microbenchmark<HashTable>::register_benchmarks(benchmarks);

	// Register instrumentations
	common::contender_list<common::instrumentation> instrumentations;
	instrumentations.register_contender("timer", "timer",
		[](){ return new common::timer_instrumentation(); },
		[](common::instrumentation* instr) { delete (common::timer_instrumentation*) instr; });

	instrumentations.register_contender("PAPI cache", "PAPI_cache",
		[](){ return new common::papi_instrumentation_cache(); },
		[](common::instrumentation* instr) { delete (common::papi_instrumentation_cache*) instr; });

	instrumentations.register_contender("PAPI instruction", "PAPI_instr",
		[](){ return new common::papi_instrumentation_instr(); },
		[](common::instrumentation* instr) { delete (common::papi_instrumentation_instr*) instr; });

	// Open result file
	std::fstream res;
	res.open(resultfn, std::fstream::out);

	// Run all combinations!
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

				// Print RESULT lines for sqlplot-tools
				res << "RESULT"
					<< " ds=" << datastructure_factory.key()
					<< " bench=" << benchmark_factory.key();
				t->result(res);
				res << std::endl;

				delete benchmark;
				results.push_back(t);
			}
		}
		// TODO: aggregate and evaluate results

		// Delete results
		auto temp = instrumentation();
		temp->destroy(results.begin(), results.end());
		instrumentation.destroy(temp);
		results.clear();
		std::cout << std::endl << std::endl;
	}

	res.close();

	// Shut down PAPI if it was used
	if (PAPI_is_initialized() != PAPI_NOT_INITED)
		PAPI_shutdown();
}

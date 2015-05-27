#include <fstream>
#include <iostream>
#include <vector>

#include <papi.h>

#include "common/arg_parser.h"
#include "common/benchmark.h"
#include "common/contenders.h"
#include "common/instrumentation.h"
#include "common/terminal.h"
#include "hashtable/unordered_map.h"
#include "hashtable/microbenchmark.h"

int main(int argc, char** argv) {
	common::arg_parser args(argc, argv);
	std::string resultfn_prefix = args.get<std::string>("p", "results_");

	using HashTable = hashtable::hashtable<int, int>;
	using Benchmark = common::benchmark<HashTable, size_t>;

	// Set up data structure contenders
	common::contender_list<HashTable> contenders;
	hashtable::unordered_map<int, int>::register_contenders(contenders);

	// Register Benchmarks
	common::contender_list<Benchmark> benchmarks;
	hashtable::microbenchmark<HashTable>::register_benchmarks(benchmarks);

	// Register instrumentations
	common::contender_list<common::instrumentation> instrumentations;
#ifndef MALLOC_INSTR
	instrumentations.register_contender("timer", "timer",
		[](){ return new common::timer_instrumentation(); });

	instrumentations.register_contender("PAPI cache", "PAPI_cache",
		[](){ return new common::papi_instrumentation_cache(); });

	instrumentations.register_contender("PAPI instruction", "PAPI_instr",
		[](){ return new common::papi_instrumentation_instr(); });
#else
	instrumentations.register_contender("memory usage", "memory",
		[](){ return new common::memory_instrumentation(); });
#endif

	// Open result file
	std::fstream res;

	std::vector<std::vector<common::benchmark_result*>> results;
	results.resize(instrumentations.size());

	auto result_it = results.begin();

	// Run all combinations!
	for (auto instrumentation : instrumentations) {
		res.open(resultfn_prefix + instrumentation.key() + ".txt", std::fstream::out);
		std::cout << term_bold << common::term_set_colour(common::term_colour::fg_yellow)
				  << "Running benchmark with " << instrumentation.description() << " instrumentation"
				  << term_reset << std::endl;

		std::vector<common::benchmark_result*> &instr_results = *(result_it++);

		for (auto datastructure_factory : contenders) {
			for (auto benchmark_factory : benchmarks) {
				auto benchmark = benchmark_factory();
				// dry run with first configuration to prevent skews
				auto initial_configuration = *(benchmark->begin());
				benchmark->run(datastructure_factory, instrumentation,
					initial_configuration, benchmark_factory);

				// Run benchmark on all configurations
				for (auto configuration : *benchmark) {
					// We need to pass in the benchmark factory here so that the
					// result object can know its name...
					auto t = benchmark->run(datastructure_factory, instrumentation,
						configuration, benchmark_factory);
					std::cout << *t << std::endl;

					// Print RESULT lines for sqlplot-tools
					res << "RESULT"
						<< " config=" << configuration
						<< " ds=" << datastructure_factory.key()
						<< " bench=" << benchmark_factory.key();
					t->result(res);
					res << std::endl;

					instr_results.push_back(t);
				}
				delete benchmark;
				std::cout << std::endl;
			}
			std::cout << std::endl;
		}
		res.close();
		// TODO: aggregate and evaluate results

	}

	result_it = results.begin();
	for (auto instrumentation : instrumentations) {
		// Delete results
		std::vector<common::benchmark_result*> &instr_results = *(result_it++);
		auto temp = instrumentation();
		temp->destroy(instr_results.begin(), instr_results.end());
		instrumentation.destroy(temp);
		instr_results.clear();
	}

	// Shut down PAPI if it was used
	if (PAPI_is_initialized() != PAPI_NOT_INITED)
		PAPI_shutdown();
}

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
	size_t repetitions = args.get<size_t>("n", 1);

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

	std::vector<std::vector<common::benchmark_result_aggregate>> results;
	results.reserve(contenders.end() - contenders.begin());

	// Run all combinations!
	bool first_iteration = true;
	for (auto datastructure_factory : contenders) {
		std::cout << term_bold << term_underline << common::term_set_colour(common::term_colour::fg_green)
				  << "Benchmarking " << datastructure_factory.description()
				  << term_reset << std::endl;

		std::fstream::openmode res_flags = std::fstream::out;
		// overwrite on first iteration, append afterwards
		if (!first_iteration) res_flags |= std::fstream::app;
		else first_iteration = false;

		std::vector<common::benchmark_result_aggregate> ds_results;

		for (auto instrumentation_factory : instrumentations) {
			std::cout << term_bold << common::term_set_colour(common::term_colour::fg_yellow)
					  << "Benchmarking " << datastructure_factory.description()
					  << " with " << instrumentation_factory.description() << " instrumentation"
					  << term_reset << std::endl;

			std::fstream res(resultfn_prefix + instrumentation_factory.key() + ".txt", res_flags);

			auto instrumentation = instrumentation_factory();

			for (auto benchmark_factory : benchmarks) {
				auto benchmark = benchmark_factory();
				// dry run with first configuration to prevent skews
				auto initial_configuration = *(benchmark->begin());
				delete benchmark->run(datastructure_factory, instrumentation,
					initial_configuration);

				// Run benchmark on all configurations
				for (auto configuration : *benchmark) {
					common::benchmark_result_aggregate aggregate(instrumentation->new_result(true),
						instrumentation->new_result(), instrumentation->new_result());

					for (size_t rep = 0; rep < repetitions; ++rep) {
						// We need to pass in the benchmark factory here so that the
						// result object can know its name...
						auto t = benchmark->run(datastructure_factory, instrumentation, configuration);
						aggregate.add_result(t);

						// Print RESULT lines for sqlplot-tools
						res << "RESULT"
							<< " config=" << configuration
							<< " ds=" << datastructure_factory.key()
							<< " bench=" << benchmark_factory.key();
						t->result(res);
						res << std::endl;
						delete t;
					}

					aggregate.finish();
					aggregate.set_properties(benchmark_factory.description(),
						datastructure_factory.description(), configuration);

					// Aggregate results of multiple runs
					std::cout << aggregate << std::endl;
					ds_results.emplace_back(std::move(aggregate));
				}
				delete benchmark;
				std::cout << std::endl;
			}
			res.close();
			delete instrumentation;
			std::cout << std::endl;
		}

		results.emplace_back(std::move(ds_results));
	}

	// TODO: compare results

	for (auto &ds_results : results)
		for (auto &result : ds_results)
			result.destroy();


	// Shut down PAPI if it was used
	if (PAPI_is_initialized() != PAPI_NOT_INITED)
		PAPI_shutdown();
}

#include <fstream>
#include <iostream>
#include <vector>

#include <papi.h>


#include "common/arg_parser.h"
#include "common/benchmark.h"
#include "common/comparison.h"
#include "common/contenders.h"
#include "common/experiments.h"
#include "common/instrumentation.h"

#include "pq/priority_queue.h"
#include "pq/std_pq.h"
#include "pq/gnu_pq.h"
#include "pq/microbenchmark.h"

void usage(char* name) {
	using std::cout;
	using std::endl;
	cout << "Usage: " << name << " <options>" << endl << endl
		 << "Options:" << endl
		 << "-o <filename> result serialization filename (default: data_pq.txt)" << endl
		 << "-p <prefix>   result filename prefix (default: results_pq_)" << endl
		 << "-n <int>      number of repetitions for each benchmark (default: 1)" << endl
		 << "-c <double>   cutoff, at which difference ratio to stop printing (deafult: 1.01)" << endl
		 << "-m <int>      maximum number of differences to print (default: 25)" << endl
		 << endl
		 << "Instrumentation options:" << endl
		 << "-nt           disable timer instrumentation" << endl
		 << "-np           disable all PAPI instrumentations" << endl
		 << "-npc          disable PAPI cache instrumentation" << endl
		 << "-npi          disable PAPI instruction instrumentation" << endl;
	exit(0);
}

int main(int argc, char** argv) {
	common::arg_parser args(argc, argv);
	if (args.is_set("h") || args.is_set("-help")) usage(argv[0]);
	const std::string resultfn_prefix = args.get<std::string>("p", "results_pq_"),
		serializationfn = args.get<std::string>("o", "data_pq.txt");
	const size_t repetitions = args.get<size_t>("n", 1),
		         max_results = args.get<size_t>("m", 25);
	const double cutoff = args.get<double>("c", 1.01);
	__attribute__((unused))
	const bool disable_timer      = args.is_set("nt"),
			   disable_papi_cache = args.is_set("npc") || args.is_set("np"),
			   disable_papi_instr = args.is_set("npi") || args.is_set("np");

	using PQ = pq::priority_queue<int>;
	using Benchmark = common::benchmark<PQ, size_t>;

	// Set up data structure contenders
	common::contender_list<PQ> contenders;
	pq::std_pq<int>::register_contenders(contenders);
#if defined(__GNUG__) && !(defined(__APPLE_CC__))
	// These are from GNU libstdc++ policy-based datastructures library
	pq::gnu_pq<int>::register_contenders(contenders);
#endif

	// Register Benchmarks
	common::contender_list<Benchmark> benchmarks;
	pq::microbenchmark<PQ>::register_benchmarks(benchmarks);

	// Register instrumentations
	common::contender_list<common::instrumentation> instrumentations;
#ifndef MALLOC_INSTR
	if (!disable_timer)
	instrumentations.register_contender("timer", "timer",
		[](){ return new common::timer_instrumentation(); });

	if (!disable_papi_cache)
	instrumentations.register_contender("PAPI cache", "PAPI_cache",
		[](){ return new common::papi_instrumentation_cache(); });

	if (!disable_papi_instr)
	instrumentations.register_contender("PAPI instruction", "PAPI_instr",
		[](){ return new common::papi_instrumentation_instr(); });
#else
	instrumentations.register_contender("memory usage", "memory",
		[](){ return new common::memory_instrumentation(); });
#endif

	std::vector<std::vector<common::benchmark_result_aggregate>> results;

	common::experiment_runner<PQ, size_t> runner(contenders, instrumentations, benchmarks, results);
	runner.run(repetitions, resultfn_prefix);

	if (contenders.size() > 1) {
		common::comparison comparison(results, 0);
		comparison.compare();
		comparison.print(std::cout, cutoff, max_results);
	}

	// Serialize results
	runner.serialize(serializationfn);

	runner.shutdown();
}

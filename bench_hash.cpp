#include <fstream>
#include <iostream>
#include <vector>

#include <papi.h>

#include "common/arg_parser.h"
#include "common/benchmark.h"
#include "common/comparison.h"
#include "common/contenders.h"
#include "common/experiments.h"
#include "common/hack.h"
#include "common/instrumentation.h"

#include "hashtable/dense_hash_map.h"
#include "hashtable/sparse_hash_map.h"
#include "hashtable/unordered_map.h"
#include "hashtable/microbenchmark.h"
#include "hashtable/wordcount.h"

void usage(char* name) {
    using std::cout;
    using std::endl;
    cout << "Usage: " << name << " <options>" << endl << endl
         << "Options:" << endl
         << "-a            append results instead of replacing" << endl
         << "-o <filename> result serialization filename (default: data_hash.txt)" << endl
         << "-p <prefix>   result filename prefix (default: results_hash_)" << endl
         << "-n <int>      number of repetitions for each benchmark (default: 1)" << endl
         << "-c <double>   cutoff, at which difference ratio to stop printing (deafult: 1.01)" << endl
         << "-m <int>      maximum number of differences to print (default: 25)" << endl
         << "-b <int>      which contender to compare to the others (default: 0)" << endl
         << endl
         << "Instrumentation options:" << endl
         << "-nt           disable timer instrumentation" << endl
         << "-np           disable all PAPI instrumentations" << endl
         << "-npc          disable PAPI cache instrumentation" << endl
         << "-npi          disable PAPI instruction instrumentation" << endl;
    exit(0);
}

int main(int argc, char** argv) {
    // Parse command-line arguments
    common::arg_parser args(argc, argv);
    if (args.is_set("h") || args.is_set("-help")) usage(argv[0]);
    const std::string resultfn_prefix = args.get<std::string>("p", "results_hash_"),
                      serializationfn = args.get<std::string>("o", "data_hash.txt");
    const int repetitions    = args.get<int>("n", 1),
              max_results    = args.get<int>("m", 25),
              base_contender = args.get<int>("b", 0);
    const double cutoff = args.get<double>("c", 1.01);
    __attribute__((unused)) // don't warn when compiling malloc target
    const bool disable_timer      = args.is_set("nt"),
               disable_papi_cache = args.is_set("npc") || args.is_set("np"),
               disable_papi_instr = args.is_set("npi") || args.is_set("np"),
               append_results = args.is_set("a");

    using HashTable = hashtable::hashtable<int, int>;
    using Configuration = std::pair<size_t, size_t>;
    using Benchmark = common::benchmark<HashTable, Configuration>;

    // Set up data structure contenders
    common::contender_list<HashTable> contenders;
    // TODO: add your own implemenation here!

    // Add wrappers around std::unordered_map and Google's libsparsehash
    hashtable::unordered_map<int, int>::register_contenders(contenders);
    hashtable::dense_hash_map<int, int>::register_contenders(contenders);
    hashtable::sparse_hash_map<int, int>::register_contenders(contenders);

    // Register Benchmarks
    common::contender_list<Benchmark> benchmarks;
    hashtable::microbenchmark<HashTable>::register_benchmarks(benchmarks);
    hashtable::wordcount<HashTable>::register_benchmarks(benchmarks);

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

    // Run the benchmarks
    common::experiment_runner<HashTable, Configuration> runner(contenders, instrumentations, benchmarks, results);
    runner.run(repetitions, resultfn_prefix);

    // Evaluate the result
    if (contenders.size() > 1) {
        common::comparison comparison(results, base_contender);
        comparison.compare();
        comparison.print(std::cout, cutoff, max_results);
    }

    // Serialize results to disk for further evaluation
    runner.serialize(serializationfn, append_results);

    runner.shutdown();
}

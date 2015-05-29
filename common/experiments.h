#pragma once

#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

#include "benchmark.h"
#include "contenders.h"
#include "instrumentation.h"
#include "terminal.h"

namespace common {

template <typename DataStructure, typename Configuration>
class experiment_runner {
public:
    using Benchmark = common::benchmark<DataStructure, Configuration>;

    experiment_runner(
            common::contender_list<DataStructure> &contenders,
            common::contender_list<common::instrumentation> &instrumentations,
            common::contender_list<Benchmark> &benchmarks,
            std::vector<std::vector<common::benchmark_result_aggregate>> &results)
        : contenders(contenders)
        , instrumentations(instrumentations)
        , benchmarks(benchmarks)
        , results(results)
    {
        results.reserve(contenders.end() - contenders.begin());
    }

    void run(size_t repetitions, const std::string &resultfn_prefix) {
        // Run all combinations!
        bool first_iteration = true;
        for (auto datastructure_factory : contenders) {
            std::cout << term::bold << term::underline << common::term::set_colour(common::term::colour::fg_green)
                      << "Benchmarking " << datastructure_factory.description()
                      << term::reset << std::endl;

            std::fstream::openmode res_flags = std::fstream::out;
            // overwrite on first iteration, append afterwards
            if (!first_iteration) res_flags |= std::fstream::app;
            else first_iteration = false;

            std::vector<common::benchmark_result_aggregate> ds_results;

            for (auto instrumentation_factory : instrumentations) {
                std::cout << term::bold << common::term::set_colour(common::term::colour::fg_yellow)
                          << "Benchmarking " << datastructure_factory.description()
                          << " with " << instrumentation_factory.description() << " instrumentation"
                          << term::reset << std::endl;

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
                            datastructure_factory.description(), configuration,
                            instrumentation_factory.description());

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
    }

    void serialize(const std::string &fn) {
        std::ofstream ofs(fn);
        boost::archive::text_oarchive oa(ofs);
        oa << results;
    }

    void shutdown() {
        // TODO use smart pointers
        for (auto &ds_results : results)
            for (auto &result : ds_results)
                result.destroy();


        // Shut down PAPI if it was used
        if (PAPI_is_initialized() != PAPI_NOT_INITED)
            PAPI_shutdown();
    }
protected:
    common::contender_list<DataStructure> &contenders;
    common::contender_list<common::instrumentation> &instrumentations;
    common::contender_list<Benchmark> &benchmarks;
    std::vector<std::vector<common::benchmark_result_aggregate>> &results;
};

}

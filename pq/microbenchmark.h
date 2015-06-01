#pragma once

#include <random>
#include <type_traits>
#include <vector>
#include <utility>

#include "../common/benchmark.h"
#include "../common/benchmark_util.h"
#include "../common/contenders.h"

namespace pq {

template <typename PQ>
class microbenchmark {
public:
    using Configuration = std::pair<size_t, size_t>;
    using Benchmark = common::benchmark<PQ, Configuration>;
    using BenchmarkFactory = common::contender_factory<Benchmark>;
    using T = typename PQ::value_type;

    static void* fill_data_permutation(PQ&, Configuration config, void*) {
        return common::util::fill_data_permutation<T>(
            config.first, config.second);
    }

    template <int factor=1>
    static void* fill_data_random(PQ&, Configuration config, void*) {
        return common::util::fill_data_random<T>(
            factor*config.first, config.second);
    }

    template <int factor = 1>
    static void* fill_heap_random(PQ& queue, Configuration config, void*) {
        std::mt19937 random{config.second};
        size_t size = factor * config.first;
        for (size_t i = 0; i < size; ++i)
            queue.push(random());
        return nullptr;
    }

    template <int qfactor=1, int dfactor=1>
    static void* fill_both_random(PQ &queue, Configuration config, void* data) {
        fill_heap_random<qfactor>(queue, config, data);
        config.second++; // "new" seed
        return fill_data_random<dfactor>(queue, config, data);
    }

    static void clear_data(PQ&, Configuration, void* data) {
        common::util::delete_data<T>(data);
    }

    static void register_benchmarks(common::contender_list<Benchmark> &benchmarks) {
        const std::vector<Configuration> configs{
            std::make_pair(1<<16, 0xDECAF),
            std::make_pair(1<<18, 0xBEEF),
            std::make_pair(1<<20, 0xC0FFEE),
            //std::make_pair(1<<22, 0xF005BA11),
            //std::make_pair(1<<24, 0xBA5EBA11),
            //std::make_pair(1<<26, 0xCA55E77E)
        };

        common::register_benchmark("push", "push",  microbenchmark::fill_heap_random<1>, configs, benchmarks);

        common::register_benchmark("pop", "pop", microbenchmark::fill_heap_random<1>,
            [](PQ &queue, Configuration, void*) {
                while (queue.size() > 0)
                    queue.pop();
            }, configs, benchmarks);

        common::register_benchmark("push-pop-mix on full heap", "push-pop-mix",
            microbenchmark::fill_both_random<1>,
            [](PQ &queue, Configuration config, void* ptr) {
                T* data = static_cast<T*>(ptr);
                for (size_t i = 0; i < config.first; ++i) {
                    queue.push(data[i]);
                    queue.pop();
                }
            }, microbenchmark::clear_data, configs, benchmarks);

        common::register_benchmark("(push-pop-push)^n (pop-push-pop)^n", "idi^n-did^n",
            microbenchmark::fill_data_random<3>,
            [](PQ &queue, Configuration config, void* ptr) {
                T* data = static_cast<T*>(ptr);
                size_t size = config.first;
                for (size_t i = 0; i < size; ++i) {
                    queue.push(data[i]);
                    queue.pop();
                    queue.push(data[size+i]);
                }
                for (size_t i = 0; i < size; ++i) {
                    queue.pop();
                    queue.push(data[2*size+i]);
                    queue.pop();
                }
            }, microbenchmark::clear_data, configs, benchmarks);
    }
};
}

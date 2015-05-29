#pragma once

#include <random>
#include <type_traits>
#include <vector>
#include <utility>

#include "../common/benchmark.h"
#include "../common/contenders.h"
#include "../common/timer.h"

namespace pq {

template <typename PQ>
class microbenchmark {
public:
    using Configuration = std::pair<size_t, size_t>;
    using Benchmark = common::benchmark<PQ, Configuration>;
    using BenchmarkFactory = common::contender_factory<Benchmark>;

    template <typename F>
    static typename PQ::value_type* fill_data(size_t size, F&& cb) {
        auto data = new typename PQ::value_type[size];
        for (size_t i = 0; i < size; ++i)
            data[i] = cb(i);
        return data;
    }

    static void* fill_data_permutation(PQ&, Configuration config, void*) {
        auto data = fill_data(config.first, [](size_t i) {return i;});
        std::shuffle(data, data + config.first, std::mt19937{config.second});
        return data;
    }

    static void* fill_data_random(PQ&, Configuration config, void*) {
        std::mt19937 gen{config.second};
        return fill_data(config.first, [&gen](size_t) {return gen();});
    }

    static void* fill_heap_random(PQ& queue, Configuration config, void*) {
        std::mt19937 random{config.second};
        for (size_t i = 0; i < config.first; ++i) {
            queue.push(random());
        }
        return nullptr;
    }

    static void register_benchmarks(common::contender_list<Benchmark> &benchmarks) {
        const std::vector<Configuration> configs{
            std::make_pair(1<<16, 1234567),
            std::make_pair(1<<18, 0xBEEF),
            std::make_pair(1<<20, 0xC0FFEE),
            /*1<<22, 1<<24, 1<<26*/};

        common::register_benchmark("insert", "insert",  microbenchmark::fill_heap_random, configs, benchmarks);

        common::register_benchmark("pop", "pop", microbenchmark::fill_heap_random, [](PQ &queue, Configuration, void*) {
            while (queue.size() > 0) {
                queue.pop();
            }
        }, configs, benchmarks);
    }
};
}

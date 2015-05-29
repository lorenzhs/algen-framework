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

    static void* fill_with_permutation(PQ&, Configuration config, void*) {
        size_t size = config.first;
        auto data = new typename PQ::value_type[size];
        for (size_t i = 0; i < size; ++i)
            data[i] = i;
        std::shuffle(data, data + size, std::mt19937{config.second});
        return data;
    }

    static void* fill_with_random(PQ&, Configuration config, void*) {
        size_t size = config.first;
        auto data = new typename PQ::value_type[size];
        std::mt19937 random{config.second};
        for (size_t i = 0; i < size; ++i)
            data[i] = random();
        return data;
    }

    static void register_benchmarks(common::contender_list<Benchmark> &benchmarks) {
        auto fill = [](PQ &queue, size_t num, void*) {
            for (size_t i = 1; i <= num; ++i) {
                queue.push(i);
            }
            return nullptr;
        };

        const std::vector<Configuration> configs{
            std::make_pair(1<<16, 1234567),
            std::make_pair(1<<18, 0xBEEF),
            std::make_pair(1<<20, 0xC0FFEE),
            /*1<<22, 1<<24, 1<<26*/};

        common::register_benchmark("insert", "insert",  fill, configs, benchmarks);

        common::register_benchmark("pop", "pop", fill, [](PQ &queue, Configuration, void*) {
            while (queue.size() > 0) {
                queue.pop();
            }
        }, configs, benchmarks);
    }
};
}

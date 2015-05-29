#pragma once

#include <utility>

#include "../common/benchmark.h"
#include "../common/contenders.h"

#include "microbenchmark.h"

namespace pq {

template <typename PQ>
class heapsort {
public:
    using Configuration = std::pair<size_t, size_t>;
    using Benchmark = common::benchmark<PQ, Configuration>;
    using BenchmarkFactory = common::contender_factory<Benchmark>;

    template<typename It>
    static void sort(PQ &heap, It begin, It end) {
        It it = begin;
        while (it != end) {
            heap.push(*it++);
        }
        it = begin;
        while (heap.size() > 0) {
            *it++ = heap.top();
            heap.pop();
        }
    }


    static void register_benchmarks(common::contender_list<Benchmark> &benchmarks) {
        const std::vector<Configuration> configs{
            std::make_pair(1<<16, 1234567),
            std::make_pair(1<<18, 0xBEEF),
            std::make_pair(1<<20, 0xC0FFEE),
            /*1<<22, 1<<24, 1<<26*/};

        common::register_benchmark("heapsort", "heapsort",
            microbenchmark<PQ>::fill_with_permutation,
            [](PQ &queue, Configuration config, void* data) {
                assert(data != nullptr);
                auto ptr = static_cast<typename PQ::value_type*>(data);
                heapsort::sort(queue, ptr, ptr+config.first);
            },
            [](PQ&, Configuration, void* data) {
                delete[] static_cast<typename PQ::value_type*>(data);
            }, configs, benchmarks);
    }
};

}

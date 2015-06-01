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

        common::register_benchmark("heapsort permutation", "heapsort-perm",
            microbenchmark<PQ>::fill_data_permutation,
            [](PQ &queue, Configuration config, void* data) {
                assert(data != nullptr);
                auto ptr = static_cast<typename PQ::value_type*>(data);
                heapsort::sort(queue, ptr, ptr+config.first);
            },
            [](PQ&, Configuration config, void* data) {
                auto ptr = static_cast<typename PQ::value_type*>(data);
                // Check that data is sorted
                for (size_t i = 0; i < config.first; ++i) {
                    assert(ptr[i] == static_cast<typename PQ::value_type>(config.first - i - 1));
                }
                delete[] ptr;
            }, configs, benchmarks);

        common::register_benchmark("heapsort random", "heapsort-rand",
            microbenchmark<PQ>::template fill_data_random<1>,
            [](PQ &queue, Configuration config, void* data) {
                assert(data != nullptr);
                auto ptr = static_cast<typename PQ::value_type*>(data);
                heapsort::sort(queue, ptr, ptr+config.first);
            }, microbenchmark<PQ>::clear_data, configs, benchmarks);
    }
};

}

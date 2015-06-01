#pragma once

#include <random>
#include <type_traits>
#include <vector>
#include <utility>

#include "../common/benchmark.h"
#include "../common/contenders.h"

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

    template <int factor=1>
    static void* fill_data_random(PQ&, Configuration config, void*) {
        std::mt19937 gen{config.second};
        return fill_data(config.first*factor, [&gen](size_t) {return gen();});
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

    static void clear_data(PQ&, Configuration, void* ptr) {
        auto data = static_cast<typename PQ::value_type*>(ptr);
        delete[] data;
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
                auto data = static_cast<typename PQ::value_type*>(ptr);
                for (size_t i = 0; i < config.first; ++i) {
                    queue.push(data[i]);
                    queue.pop();
                }
            }, microbenchmark::clear_data, configs, benchmarks);

        common::register_benchmark("(push-pop-push)^n (pop-push-pop)^n", "idi^n-did^n",
            microbenchmark::fill_data_random<3>,
            [](PQ &queue, Configuration config, void* ptr) {
                auto data = static_cast<typename PQ::value_type*>(ptr);
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

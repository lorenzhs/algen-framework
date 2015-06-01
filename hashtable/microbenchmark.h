#pragma once

#include <type_traits>
#include <utility>

#include "../common/benchmark.h"
#include "../common/benchmark_util.h"
#include "../common/contenders.h"

namespace hashtable {

template <typename HashTable>
class microbenchmark {
public:
    using Configuration = std::pair<size_t, size_t>;
    using Benchmark = common::benchmark<HashTable, Configuration>;
    using BenchmarkFactory = common::contender_factory<Benchmark>;
    using T = typename HashTable::mapped_type;
    common::contender_list<Benchmark> benchmarks;

    template <int factor=1>
    static void* fill_data_random(HashTable&, Configuration config, void*) {
        return common::util::fill_data_random<T>(
            factor*config.first, config.second);
    }

    static void* fill_map_random(HashTable &map, Configuration config, void*) {
        std::mt19937 gen{config.second};
        for (size_t i = 0; i < config.first; ++i) {
            map[i] = gen();
        }
        return nullptr;
    }

    template <int factor = 1>
    static void* fill_both_random(HashTable &map, Configuration config, void* ptr) {
        fill_map_random(map, config, ptr);
        config.second++;
        return fill_data_random<factor>(map, config, ptr);
    }

    static void delete_data(HashTable&, Configuration, void* data) {
        common::util::delete_data<T>(data);
    }

    static void register_benchmarks(common::contender_list<Benchmark> &benchmarks) {
        auto fill = [](HashTable &map, Configuration config, void* ptr) {
            T* data = static_cast<T*>(ptr);
            for (size_t i = 1; i <= config.first; ++i) {
                map[i] = data[i];
            }
            return nullptr;
        };

        const std::vector<Configuration> configs{
            std::make_pair(1<<16, 0xDECAF),
            std::make_pair(1<<18, 0xBEEF),
            std::make_pair(1<<20, 0xC0FFEE),
            //std::make_pair(1<<22, 0xF005BA11),
            //std::make_pair(1<<24, 0xBA5EBA11),
            //std::make_pair(1<<26, 0xCA55E77E)
        };

        // insert data
        common::register_benchmark("insert", "insert",  microbenchmark::fill_data_random<1>,
            fill, microbenchmark::delete_data, configs, benchmarks);

        // insert elements and find them
        common::register_benchmark("insert+find", "insert-find", microbenchmark::fill_data_random<1>,
            [](HashTable &map, Configuration config, void* ptr) {
                T* data = static_cast<T*>(ptr);
                size_t num = config.first;
                for (size_t i = 1; i <= num; ++i) {
                    map[i] = data[i];
                }
                for (size_t i = 1; i <= num; ++i) {
                    map.find(i);
                }
            }, microbenchmark::delete_data, configs, benchmarks);

        // insert-delete-insert delete-insert-delete cycles
        common::register_benchmark("(ins-del-ins)^n (del-ins-del)^n", "ins-del-cycle", microbenchmark::fill_data_random<3>,
            [](HashTable &map, Configuration &config, void* ptr) {
                T* data = static_cast<T*>(ptr);
                size_t num = config.first;
                for (size_t i = 1; i <= num; ++i) {
                    map[i] = data[i];
                    map.erase(i);
                    map[i] = data[num + i];
                }
                for (size_t i = 1; i <= num; ++i) {
                    map.erase(i);
                    map[i] = data[2*num + i];
                    map.erase(i);
                }
            }, microbenchmark::delete_data, configs, benchmarks);

        // access entries that were previously inserted
        common::register_benchmark("access", "access", microbenchmark::fill_map_random,
            [](HashTable &map, Configuration config, void*) {
                for (size_t i = 1; i <= config.first; ++i) {
                    (void)map[i];
                }
            }, configs, benchmarks);

        // find entries that were previously inserted
        common::register_benchmark("find", "find", microbenchmark::fill_map_random,
            [](HashTable &map, Configuration config, void*) {
                for (size_t i = 1; i <= config.first; ++i) {
                    (void)map.find(i);
                }
            }, configs, benchmarks);

        // find random keys that very likely don't exist
        common::register_benchmark("find random", "find-random", microbenchmark::fill_both_random<1>,
            [](HashTable &map, Configuration config, void* ptr) {
                T* data = static_cast<T*>(ptr);
                for (size_t i = 1; i <= config.first; ++i) {
                    (void)map.find(data[i]);
                }
            }, microbenchmark::delete_data, configs, benchmarks);
    }
};
}

#pragma once

#include <cassert>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

#include "../common/benchmark.h"
#include "../common/contenders.h"

namespace hashtable {

template <typename HashTable>
class wordcount {
public:
    using Configuration = std::pair<size_t, size_t>;
    using Benchmark = common::benchmark<HashTable, Configuration>;
    using BenchmarkFactory = common::contender_factory<Benchmark>;

    // fake word count, doesn't actually determine the most frequent
    // words because our hashtables don't have an iterator interface
    template <typename It>
    static void count(HashTable &map, It begin, It end) {
        It it = begin;
        while (it != end) {
            map[*it++]++;
        }
    }

    static void register_benchmarks(common::contender_list<Benchmark> &benchmarks) {
        // HACKHACKHACK
        const std::vector<Configuration> configs{
            std::make_pair(0x4b61666b61, 0x56657277616e646c), // "Kafka", "Verwandl"
            std::make_pair(0x5368616b657370, 0x636f6d706c657465) // "Shakesp", "complete"
        };

        using Key = typename HashTable::key_type;

        common::register_benchmark("wordcount", "wordcount",
            [](HashTable&, Configuration config, void*) -> void* {
                // awful hack approaching
                std::stringstream fn;
                // convert encoded filename back to ascii
                fn << "data/wordcount_" << common::util::hex_to_ascii(config.first);
                if (config.second > 0)
                    fn << "_" << common::util::hex_to_ascii(config.second);
                fn << ".txt";

                std::ifstream in(fn.str());
                if (!in.is_open())
                    throw std::invalid_argument("Cannot open file '" + fn.str() + "'.");

                // map strings to key type because stupid benchmark
                std::unordered_map<std::string, Key> ids;
                ids[""] = Key{}; // dummy to use key Key{}
                auto words = new std::vector<Key>();

                std::string word;
                while (in >> word) {
                    Key& key = ids[word];
                    if (key == Key{}) {
                        key = static_cast<Key>(ids.size());
                    }
                    words->push_back(key);
                }

                // Allocate data array and copy
                return words;
            },
            [](HashTable &map, Configuration, void* ptr) {
                assert(ptr != nullptr);
                auto data = static_cast<std::vector<Key>*>(ptr);
                wordcount::count(map, data->begin(), data->end());
            },
            [](HashTable &, Configuration, void* ptr) {
                delete static_cast<std::vector<Key>*>(ptr);
            }, configs, benchmarks);
    }

};

}
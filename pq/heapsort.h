#pragma once

namespace pq {

template <typename PQ>
class heapsort {
public:
    using Benchmark = common::benchmark<PQ, size_t>;
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
        const std::vector<size_t> sizes{1<<16, 1<<18, 1<<20, /*1<<22, 1<<24, 1<<26*/};
        
        common::register_benchmark("heapsort", "heapsort", [](PQ&, size_t size, void*) {
            return new typename PQ::value_type[size];
            // TODO: add predictable pseudorandom data
        }, [](PQ &queue, size_t size, void* data) {
            assert(data != nullptr);
            auto ptr = static_cast<typename PQ::value_type*>(data);
            heapsort::sort(queue, ptr, ptr+size);
        }, [](PQ&, size_t, void* data) {
            delete[] static_cast<typename PQ::value_type*>(data);
        }, sizes, benchmarks);
    }
};

}

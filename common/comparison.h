#pragma once

#include <algorithm>
#include <cassert>
#include <cmath>
#include <limits>
#include <tuple>
#include <vector>

#include "benchmark.h"
#include "terminal.h"

namespace common {

class comparison {
public:
    using result = benchmark_result_aggregate;
    comparison(std::vector<std::vector<result>> &results, size_t base_index)
        : results(results), similarity(), base_index(base_index)
    {
        assert(base_index < results.size());
        __attribute__((unused))
        size_t base_size = results[base_index].size();
        for (auto &res : results) {
            assert(res.size() == base_size);
            (void)res;
        }
    }

    void compare() {
        size_t size = results[0].size();
        for (size_t i = 0; i < results.size(); ++i) {
            if (i == base_index) continue;
            for (size_t j = 0; j < size; ++j) {
                auto similarities = results[base_index][j].compare_to(results[i][j]);
                int index = 0;
                for (double sim : similarities) {
                    // if 0, nan, inf
                    if (std::fpclassify(sim) != FP_NORMAL)
                        sim = std::numeric_limits<double>::infinity();
                    else if (sim < 1)
                        // swap around order of comparison
                        sim = 1/sim;
                    similarity.push_back(std::make_tuple(i, j, sim, index++));
                };
            }
        }
        // Sort by similarity
        std::sort(similarity.begin(), similarity.end(), [](auto l, auto r) {
            return std::get<2>(l) > std::get<2>(r);
        });
    }

    std::ostream& print(std::ostream &os, double cutoff = 1.01, size_t max_results = 20) const {
        std::cout << "Comparison of " << term::bold << results[base_index][0].instance_desc() << term::reset
                  << " versus ";
        for (size_t i = 0; i < results.size(); ++i) {
            if (i == base_index) continue;
            std::cout << term::bold << results[i][0].instance_desc() << term::reset << "; ";
        }
        std::cout << "largest differences:" << std::endl;
        size_t num_results(0);
        for (auto res : similarity) {
            size_t index = std::get<0>(res);
            size_t elem = std::get<1>(res);
            double sim = std::get<2>(res);
            int subindex = std::get<3>(res);

            if (sim < cutoff || ++num_results > max_results) break;

            os << "Similarity ratio " << term::bold;
            // set appropriate colour
            if (sim > 2)         os << term::set_colour(term::colour::fg_red);
            else if (sim > 1.3)  os << term::set_colour(term::colour::fg_yellow);
            else if (sim > 1.1)  os << term::set_colour(term::colour::fg_green);
            else if (sim > 1.02) os << term::set_colour(term::colour::fg_cyan);
            // re-compare to get the exact ratio, not the one used for sorting
            os << results[base_index][elem].compare_to(results[index][elem])[subindex]
               << term::reset << " (factor " << sim << ") for ";
            results[index][elem].describe(os);
            results[index][elem].print_component(subindex, os) << "; " << term::bold
                << results[base_index][elem].instance_desc() << term::reset << ": ";
            results[base_index][elem].print_component(subindex, os) << std::endl;
        }
        if (num_results == 0) {
            std::cout << "No differences worth mentioning (cutoff = " << cutoff << ")" << std::endl;
        }
        return os;
    }

protected:
    std::vector<std::vector<result>> &results;
    std::vector<std::tuple<size_t, size_t, double, int>> similarity;
    size_t base_index;
};
}

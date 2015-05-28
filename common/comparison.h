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
	comparison(std::vector<result> &first, std::vector<result> &second)
		: first(first), second(second), similarity()
	{
		assert(first.size() == second.size());
	}

	void compare() {
		for (size_t i = 0; i < first.size(); ++i) {
			auto similarities = first[i].compare_to(second[i]);
			int index = 0;
			for (double sim : similarities) {
				// if 0, nan, inf
				if (std::fpclassify(sim) != FP_NORMAL)
					sim = std::numeric_limits<double>::infinity();
				else if (sim < 1)
					// swap around order of comparison
					sim = 1/sim;
				similarity.push_back(std::make_tuple(i, sim, index++));
			};
		}
		// Sort by similarity
		std::sort(similarity.begin(), similarity.end(), [](auto l, auto r) {
			return std::get<1>(l) > std::get<1>(r);
		});
	}

	std::ostream& print(std::ostream &os, double cutoff = 1.01, size_t max_results = 20) const {
		std::cout << "Comparison of " << term::bold << first[0].instance_desc() << term::reset
				  << " and " << term::bold << second[0].instance_desc() << term::reset << ":" << std::endl;
		size_t num_results(0);
		for (auto res : similarity) {
			size_t index = std::get<0>(res);
			double sim = std::get<1>(res);
			int subindex = std::get<2>(res);

			if (sim < cutoff || ++num_results > max_results) break;

			os << "Similarity ratio " << term::bold;
			// set appropriate colour
			if (sim > 2)         os << term::set_colour(term::colour::fg_red);
			else if (sim > 1.3)  os << term::set_colour(term::colour::fg_yellow);
			else if (sim > 1.1)  os << term::set_colour(term::colour::fg_green);
			else if (sim > 1.02) os << term::set_colour(term::colour::fg_cyan);
			os << sim << term::reset << " for result " << index << " part " << subindex << ": ";
			first[index].describe(os);
			first[ index].print_component(subindex, os) << " vs " << term::bold
				<< second[index].instance_desc() << term::reset << ": ";
			second[index].print_component(subindex, os) << std::endl;
		}
		if (num_results == 0) {
			std::cout << "No differences worth mentioning (cutoff = " << cutoff << ")" << std::endl;
		}
		return os;
	}

protected:
	std::vector<result> &first, &second;
	std::vector<std::tuple<size_t, double, int>> similarity;
};
}

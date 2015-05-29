
#include <cassert>
#include <fstream>
#include <vector>

#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

#include "common/arg_parser.h"
#include "common/benchmark.h"
#include "common/comparison.h"
#include "common/instrumentation.h"

int main(int argc, char** argv) {
	common::arg_parser args(argc, argv);
	const int max_results = args.get<int>("m", 25),
			  base_contender = args.get<int>("b", 0);
	const double cutoff = args.get<double>("c", 1.01);
	const std::string &filename = args.get<std::string>("i", "data.txt");

	std::vector<std::vector<common::benchmark_result_aggregate>> results;

	// Serialize results
	std::ifstream ifs(filename);
	assert(ifs.good());
	boost::archive::text_iarchive ia(ifs);
	ia >> results;

	common::comparison comparison(results, base_contender);
	comparison.compare();
	comparison.print(std::cout, cutoff, max_results);
}

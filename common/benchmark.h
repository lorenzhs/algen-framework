#pragma once

#include <functional>
#include <type_traits>

#include "contenders.h"

namespace common {

struct benchmark_result {
	virtual std::ostream& print(std::ostream &os) const = 0;
	friend std::ostream& operator<<(std::ostream &os, const benchmark_result &res) {
		return res.print(os);
	}
};

template <typename DataStructure>
class benchmark {
public:
	using F = std::function<void(DataStructure&&)>;
	F function, setup, teardown;

	benchmark(F &&function) : function(std::forward<F>(function)) {};
	benchmark(F &&setup, F &&function) : function(std::forward<F>(function)), setup(std::forward<F>(setup)) {};
	benchmark(F &&setup, F &&function, F &&teardown)
		: function(std::forward<F>(function))
		, setup(std::forward<F>(setup))
		, teardown(std::forward<F>(teardown)) {};
	benchmark(const F &other) = delete;
	benchmark() = delete;

	template <typename Instrumentation>
	auto run(contender_factory<DataStructure> &factory, contender_factory<Instrumentation> &instrumentation) -> decltype(instrumentation()->result()) {
		// Create data structure and set up benchmark
		DataStructure *instance = factory();
		if (setup) setup(std::forward<DataStructure>(*instance));

		// Set up instrumentation
		Instrumentation *instr = instrumentation();
		instr->setup();

		// Run benchmark
		function(std::forward<DataStructure>(*instance));

		// stop and destroy instrumentation
		instr->finish();
		auto result = instr->result();
		instrumentation.destroy(instr);

		// Tear down benchmark and destroy data structure
		if (teardown) teardown(std::forward<DataStructure>(*instance));
		factory.destroy(instance);

		return result;
	}
};

template <typename F, typename Benchmark>
void register_benchmark(std::string &&description, F &&f, common::contender_list<Benchmark> &benchmarks) {
	benchmarks.register_contender(std::forward<std::string>(description),
		[f = std::forward<F>(f)]() -> Benchmark* {
			return new Benchmark(f);
		}
	);
}

template <typename Setup, typename Function, typename Benchmark>
void register_benchmark(std::string &&description,
	Setup &&setup, Function &&function,
	common::contender_list<Benchmark> &benchmarks)
{
	benchmarks.register_contender(std::forward<std::string>(description),
		[  setup = std::forward<Setup   >(setup),
		function = std::forward<Function>(function)]() -> Benchmark* {
			return new Benchmark(setup, function);
		}
	);
}

template <typename Setup, typename Function, typename Teardown, typename Benchmark>
void register_benchmark(std::string &&description,
	Setup &&setup, Function &&function, Teardown &&teardown,
	common::contender_list<Benchmark> &benchmarks)
{
	benchmarks.register_contender(std::forward<std::string>(description),
		[  setup = std::forward<Setup   >(setup),
		function = std::forward<Function>(function),
	    teardown = std::forward<Teardown>(teardown)]() -> Benchmark* {
			return new Benchmark(setup, function, teardown);
		}
	);
}

}
